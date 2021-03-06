(function($) {

    function number_format(number, decimals, dec_point, thousands_sep) {
        // *     example: number_format(1234.56, 2, ',', ' ');
        // *     return: '1 234,56'
        number = (number + '').replace(',', '').replace(' ', '');
        var n = !isFinite(+number) ? 0 : +number,
            prec = !isFinite(+decimals) ? 0 : Math.abs(decimals),
            sep = (typeof thousands_sep === 'undefined') ? ',' : thousands_sep,
            dec = (typeof dec_point === 'undefined') ? '.' : dec_point,
            s = '',
            toFixedFix = function(n, prec) {
                var k = Math.pow(10, prec);
                return '' + Math.round(n * k) / k;
            };
        // Fix for IE parseFloat(0.55).toFixed(0) = 0;
        s = (prec ? toFixedFix(n, prec) : '' + Math.round(n)).split('.');
        if (s[0].length > 3) {
            s[0] = s[0].replace(/\B(?=(?:\d{3})+(?!\d))/g, sep);
        }
        if ((s[1] || '').length < prec) {
            s[1] = s[1] || '';
            s[1] += new Array(prec - s[1].length + 1).join('0');
        }
        return s.join(dec);
    }


    /*****************+
     * 
     * templates
     * 
     *****************/
    var templates = {
        log_rows: `
			<tr>
			<td scope="row" class="nobr">$date</td>
			<td>$type</td>
			<td class="nobr">$msg</td>
			</tr>`,
        message_box: `<div class="alert alert-danger alert-dismissible fade show" role="alert">
		<button type="button" class="close" data-dismiss="alert" aria-label="Close">
		<span aria-hidden="true">&times;</span></button>
		$message
		</div>`
    };
    /*****************+
     * 
     * Pages
     * 
     *****************/
    var appName = "RV Scale"

    function Page(el, options = null) {

        var defaults = {};
        var settings = $.extend({}, defaults, options);

        this.setup = function(el, settings) {
            this.active = null;
            this.timer = null;
            this.el = el;
            this.modal = null;
            this.settings = settings;
            $(".appname").html(appName);

        }

        this.messageBox = function(message) {
            $("#outputLog").append(templates.message_box.replace(/\$message/g, message));
        }

        this.progress = function() {
            this.loading += 10;
            this.loading = this.loading > 100 ? 100 : this.loading;
            $("#progressLoading").css("width", this.loading + "%");
        }

        this.loaded = function() {
            if (!this.modal) {
                $("#splash").addClass("d-none");
                $(this.el).removeClass("d-none");
                clearInterval(this.progressInterval);
                this.progressInterval = 0;
            }
        }

        this.formLoad = function(form, onload = null) {
            if (this.timer) {
                clearInterval(this.timer);
                this.timer = null;
            }
            if (this.modal)
                $(this.modal).modal("hide");
            this.modal = form;
            $(this.el).addClass("d-none");
            $("#splash").addClass("d-none");
            $("#splash-init").addClass("d-none");
            $("#pageDisconnected").addClass("d-none");
            if (onload) onload();
            $(form).modal("show");
        }

        this.pageLoad = function(page, onload = null) {
            if (this.timer) {
                clearInterval(this.timer);
                this.timer = null;
            }
            if (this.modal) {
                $(this.modal).modal("hide");
                this.modal = null;
            }
            this.loading = 0;
            $(this.el).addClass("d-none");
            this.progress();
            $("#splash-init").addClass("d-none");
            $("#pageDisconnected").addClass("d-none");
            $("#splash").removeClass("d-none");
            var me = this;
            this.progressInterval = setInterval(function() { me.progress(); }, 1000);
            $(this.el).load(page, onload);
        }


        this.job = function(job, time) {
            if (this.timer) {
                clearInterval(this.timer);
                this.timer = null;
            }
            this.timer = setInterval(job, time);
        }

        this.activateForm = function(form, onload = null, item = null) {
            $(".nav-item").removeClass("active");
            if (item)
                $(item).parent().addClass("active");
            this.formLoad(form, onload);
        }

        this.activatePage = function(page, onload = null, item = null) {
            $(".nav-item").removeClass("active");
            if (item)
                $(item).parent().addClass("active");
            this.pageLoad(page, onload);
        }


        this.loadDashBoard = function(item = null) {
                var me = this;
                this.activatePage("dashboard.html", function() { $(me.el).dashboard(); }, item)
            }
            /*
                    this.loadLog = function(item) {
                        var me = this;
                        this.activatePage("log.html", function() { $(me.el).logs(); }, item)
                    }
            */
        this.loadWIFI = function(item) {
            var me = this;
            this.activatePage("wifi.html", function() { $(me.el).wifi(); }, item)
        }

        this.setup(el, settings);
        var me = this;
        var timer = setInterval(function() {
                me.loadDashBoard();
            },
            1000);
    }

    $(document).ready(function() {
        window.page = new Page($("#pageContent")[0], {});
        window.page.isLogged();

    });

    /*****************+
     * 
     * DashBoard
     * 
     *****************/

    function checkRange(v, min, max) {
        return v >= min && v <= max;
    }

    function DashBoardPage(el, settings) {

        function enabled(el) {
            $(el).prop("disabled", false);
            $(el).addClass("btn-primary");
            $(el).removeClass("btn-warning")
        }

        function disabled(el, time) {
            $(el).prop("disabled", true);
            $(el).removeClass("btn-primary");
            $(el).addClass("btn-warning")
            setTimeout(function() {
                enabled(el);
            }, time)
        }

        this.loop = function() {
            var me = this;
            $.ajax({ url: "get_input.php" })
                .done(function(resp) {
                    var now = new Date();
                }).fail(function(e, x, r) {
                    if (!me.disconnected) {
                        me.disconnected = true;
                        $("#pageContent").addClass("d-none");
                        $("#pageDisconnected").removeClass("d-none");
                    }
                });
        };

        this.output = function(el, cmd, value = 1) {
            disabled(el, value * 1000);
            $.ajax({ url: "set_output.php", data: { output: cmd, value: value }, method: "POST" })
                .done(function(resp) {}).fail(function(e, x, r) {
                    if (!me.disconnected) {
                        me.disconnected = true;
                        $("#pageContent").addClass("d-none");
                        $("#pageDisconnected").removeClass("d-none");
                    }
                });
        }

        this.setup = function(el, settings) {
            this.timer = null;
            this.el = el;
            this.settings = settings;
            this.disconnected = false;
            var me = this;
            window.page.job(function() {
                me.loop();
            }, this.settings.loopInterval);
            window.page.loaded();
        };
        this.setup(el, settings);
    }

    $.fn.dashboard = function(options = {}) {
        var defaults = { loopInterval: 500 };
        var settings = $.extend({}, defaults, options);
        if (this.length == 1) {
            window.page.active = new DashBoardPage(this[0], settings);
        }
    }


    /*
     * 
     *	WIFI
     * 
     * 
     */
    function WiFiPage(el, settings) {

        this.scan = function() {
            var me = this;
            $.ajax({ url: "get_networks.php" })
                .done(function(resp) {
                    if (resp.length > 0)
                        resp = resp.substring(0, resp.length - 1);
                    var nets = resp.split("\n");
                    var rows = "";
                    for (var i = 0; i < nets.length; i++) {
                        var net = nets[i];
                        var p = net.replace(/Ch:/, "").split("|");
                        rows += "<tr><td class='netname'>" + p.join("</td><td>") + '</td><td><a class="btn btn-primary" href="#" onclick="page.active.editWIFI(this);"><span>Set</span></a></td></tr>';
                    }
                    $("#wrows").html(rows);
                    if (me.first) {
                        me.first = false;
                        window.page.loaded();
                    }
                })
        }

        this.editWIFI = function(el) {
            var netname = $(el).parent().parent().find(".netname").html();
            $("#thessid").val(netname);
            $("#thepassword").val("");
            $("#wifiModal").modal("show");
        }

        this.saveWIFI = function() {
            $.ajax({ url: "set_network.php", data: { ssid: $("#thessid").val(), key: $("#thepassword").val() } })
                .done(function(resp) { alert(resp); }).always(function() {
                    $("#wifiModal").modal("hide");
                });

        }

        this.setup = function(el, settings) {
            this.el = el;
            this.settings = settings
            this.first = true;
            this.scan();
        }

        this.setup(el, settings);
    }

    $.fn.wifi = function(options = {}) {
        var defaults = { loopInterval: 3000 };
        var settings = $.extend({}, defaults, options);

        if (this.length == 1) {
            window.page.active = new WiFiPage(this[0], settings);
        }
    }

    /*
     * 
     *	LOG
     * 
     * 
     */

    function LogPage(el, settings) {

        this.draw = function() {
            var me = this;
            $.ajax({ url: "db/log.txt" })
                .done(function(resp) {
                    if (resp)
                        resp = resp.substring(0, resp.length - 2);
                    else
                        resp = "";

                    var logs = JSON.parse("[" + resp + "]");
                    var rows = "";
                    for (var i = 0; i < logs.length; i++) {
                        var log = logs[i];
                        var p = templates.log_rows.replace(/\$date/, log.time)
                            .replace(/\$type/, log.type)
                            .replace(/\$msg/, log.msg);
                        rows += p;
                    }
                    $("#logs").html(rows);
                })
                .always(function() {
                    if (me.first) {
                        me.first = false;
                        window.page.loaded();
                    }
                })
        }

        this.setup = function(el, settings) {
            this.el = el;
            this.settings = settings
            this.first = true;
            this.draw();
        }

        this.setup(el, settings);
    }
    /*
        $.fn.logs = function(options = {}) {
            var defaults = { loopInterval: 3000 };
            var settings = $.extend({}, defaults, options);

            if (this.length == 1) {
                window.page.active = new LogPage(this[0], settings);
            }
        }
    */

})(jQuery);