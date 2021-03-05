-- MySQL dump 10.18  Distrib 10.3.27-MariaDB, for debian-linux-gnueabihf (armv8l)
--
-- Host: localhost    Database: RV
-- ------------------------------------------------------
-- Server version	10.3.27-MariaDB-0+deb10u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

create user 'rv'@'%' identified by '32camper.8';
grant all privileges to 'rv'@'%';

--
-- Table structure for table `acl`
--

DROP TABLE IF EXISTS `acl`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `acl` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `pwd` varchar(50) NOT NULL,
  `role` int(11) DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `acl`
--

LOCK TABLES `acl` WRITE;
/*!40000 ALTER TABLE `acl` DISABLE KEYS */;
/*!40000 ALTER TABLE `acl` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `history`
--

DROP TABLE IF EXISTS `history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `history` (
  `measureid` int(11) NOT NULL,
  `update_time` datetime NOT NULL,
  `v_int` int(11) DEFAULT NULL,
  `v_float` float DEFAULT NULL,
  `v_string` varchar(30) DEFAULT NULL,
  PRIMARY KEY (`measureid`,`update_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `history`
--

LOCK TABLES `history` WRITE;
/*!40000 ALTER TABLE `history` DISABLE KEYS */;
/*!40000 ALTER TABLE `history` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `measures`
--

DROP TABLE IF EXISTS `measures`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `measures` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(200) NOT NULL,
  `type` int(11) NOT NULL,
  `record` int(11) DEFAULT 0,
  `define1` varchar(10) DEFAULT NULL,
  `define2` varchar(10) DEFAULT NULL,
  `define3` varchar(10) DEFAULT NULL,
  `define4` varchar(10) DEFAULT NULL,
  `define5` varchar(10) DEFAULT NULL,
  `v_int` int(11) DEFAULT NULL,
  `v_float` float DEFAULT NULL,
  `v_string` varchar(30) DEFAULT NULL,
  `update_time` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=39 DEFAULT CHARSET=utf8mb4;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `measures`
--

LOCK TABLES `measures` WRITE;
/*!40000 ALTER TABLE `measures` DISABLE KEYS */;
INSERT INTO `measures` VALUES (1,'water/1/level',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(2,'water/2/level',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(3,'water/valve',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,'','2021-03-05 11:56:57'),(4,'water/pump',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 11:56:23'),(5,'water/1/temp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(6,'water/2/temp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(7,'water/1/unload',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(8,'water/2/unload',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(9,'sensor/int/temp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,1.36757,NULL,'2021-03-05 12:15:44'),(10,'sensor/int/hum',2,0,NULL,NULL,NULL,NULL,NULL,NULL,90.2423,NULL,'2021-03-05 12:15:45'),(11,'sensor/ext/temp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,15.6701,NULL,'2021-03-05 12:16:07'),(12,'sensor/ext/hum',2,0,NULL,NULL,NULL,NULL,NULL,NULL,57.3308,NULL,'2021-03-05 12:16:07'),(13,'battery/1/volt',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(14,'battery/1/amp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(15,'battery/1/back',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(16,'battery/2/volt',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(17,'battery/2/amp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(18,'battery/3/volt',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(19,'battery/3/amp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(20,'solar/volt',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(21,'solar/amp',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(22,'gas/1/level',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(23,'gas/2/level',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-03 12:58:17'),(24,'lights',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 11:56:23'),(25,'light/extern',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 01:49:05'),(26,'light/living',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 01:49:11'),(27,'light/kitchen',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 01:49:10'),(28,'generale',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 11:56:24'),(29,'inverter',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 11:56:49'),(30,'fridge/on',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 11:56:28'),(31,'heater/on',1,0,NULL,NULL,NULL,NULL,NULL,1,NULL,NULL,'2021-03-05 11:56:41'),(32,'airc/on',1,0,NULL,NULL,NULL,NULL,NULL,2,NULL,NULL,'2021-03-05 11:56:46'),(33,'fans/1',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(34,'fans/2',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(35,'fans/3',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(36,'fans/4',1,0,NULL,NULL,NULL,NULL,NULL,0,NULL,NULL,'2021-03-05 10:34:11'),(37,'sensor/ext/gas/1',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-05 12:16:08'),(38,'sensor/int/gas/1',2,0,NULL,NULL,NULL,NULL,NULL,NULL,0,NULL,'2021-03-05 12:15:45');
/*!40000 ALTER TABLE `measures` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-03-05 12:16:15
