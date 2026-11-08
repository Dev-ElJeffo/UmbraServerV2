-- MySQL dump 10.13  Distrib 8.0.43, for Win64 (x86_64)
--
-- Host: localhost    Database: umbra_eternum
-- ------------------------------------------------------
-- Server version	8.0.43

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `umbra_eternum`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `umbra_eternum` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;

USE `umbra_eternum`;

--
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `accounts` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `email` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `password_hash` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `salt` varchar(32) COLLATE utf8mb4_unicode_ci NOT NULL,
  `banned` tinyint(1) DEFAULT '0',
  `isadmin` tinyint(1) NOT NULL DEFAULT '0',
  `ban_reason` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_login_at` timestamp NULL DEFAULT NULL,
  `stored_gold` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Quantidade de gold armazenada no banco (compartilhado entre personagens da conta)',
  `session_version` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  UNIQUE KEY `email` (`email`),
  KEY `idx_username` (`username`),
  KEY `idx_email` (`email`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `accounts`
--

LOCK TABLES `accounts` WRITE;
/*!40000 ALTER TABLE `accounts` DISABLE KEYS */;
INSERT INTO `accounts` VALUES (1,'testuser_480','test723@example.com','REDACTED_PASSWORD_HASH','d6b3e92a3b9c1b06c7f9c47d41838082',0,0,NULL,'2025-10-14 17:08:20',NULL,0,0),(2,'testuser_491','test805@example.com','REDACTED_PASSWORD_HASH','5f2ad8a074aa354c84a12fa62de52b3c',0,0,NULL,'2025-10-14 17:26:43',NULL,0,0),(3,'testuser_749','test575@example.com','REDACTED_PASSWORD_HASH','22913ba49f80ad718bd9bf46a8de792e',1,0,'teste','2025-10-14 17:26:51',NULL,0,0),(4,'jeffo','jeffo@jeffo.com','REDACTED_PASSWORD_HASH','2c4568654b06e85930355f79b4b284b0',0,1,NULL,'2025-10-14 17:31:17','2026-07-06 18:04:24',744,161),(5,'spacce','spacce@spacce','REDACTED_PASSWORD_HASH','a2ec7f96ad0c02458c50355cceff56a9',0,0,NULL,'2025-10-14 22:36:27','2025-10-14 22:37:06',0,0),(6,'player1','player1@player','REDACTED_PASSWORD_HASH','55a8b3ef2223e1a135b92bf0a43d5237',0,0,NULL,'2025-10-15 16:52:03',NULL,0,0),(7,'player2','player2@test.com','REDACTED_PASSWORD_HASH','abe08ab40cc3dbbeea8b39f932fc44e0',0,0,NULL,'2025-10-15 18:40:30',NULL,0,0),(8,'jeffo2','jeffo@jeffo2','REDACTED_PASSWORD_HASH','abd47a22e36955804cdca9d754c48f1e',0,0,NULL,'2025-10-15 18:43:56',NULL,0,0),(9,'willzao','will@will.com','REDACTED_PASSWORD_HASH','ae9aebd79d99251d91780664571be2e8',0,0,NULL,'2025-10-15 22:51:42','2026-07-04 15:02:10',0,2),(10,'jeffito','jeffito@jeffito','REDACTED_PASSWORD_HASH','6de8f62864af207f84758ec533341cf5',0,0,NULL,'2025-10-16 15:54:12',NULL,0,0),(11,'jef','jeff@jef','REDACTED_PASSWORD_HASH','19479b66afb4abead73349631cb3a38e',0,0,NULL,'2025-10-16 15:55:49',NULL,0,0),(12,'je','jeff@je','REDACTED_PASSWORD_HASH','bcc9196d27dcb00d4158eac92e831be8',0,0,NULL,'2025-10-16 15:56:47',NULL,0,0),(13,'jefforoso','jefforoso@','REDACTED_PASSWORD_HASH','6ac790cb8908d54198345856417946b6',0,0,NULL,'2025-10-16 16:15:16',NULL,0,0),(17,'jefff','jef@','REDACTED_PASSWORD_HASH','593c4ecc27344a58a107176afc082a3f',0,0,NULL,'2025-10-16 16:22:56',NULL,0,0),(18,'asdfgh','as@as','REDACTED_PASSWORD_HASH','2fae5b248300209ec6570828d488c9c2',0,0,NULL,'2025-10-16 16:24:30','2025-10-16 16:24:39',0,0),(19,'jeffolino','jeffo@naro','REDACTED_PASSWORD_HASH','8318f28868bd1fd3027833d979a32b5c',0,0,NULL,'2025-10-16 18:07:12',NULL,0,0),(20,'bartoo','bartoo@bartoo','REDACTED_PASSWORD_HASH','2ab60f837edddda14dd7ab15cfc62513',0,0,NULL,'2025-10-16 21:40:35',NULL,0,0),(21,'sonylover','sonylover@','REDACTED_PASSWORD_HASH','f9cad1fed510ff46636512062181886a',0,0,NULL,'2025-10-16 23:51:59','2025-10-16 23:52:20',0,0),(22,'jhonny','jhonny@','REDACTED_PASSWORD_HASH','cf258657c3fbfdc6ddd4cc06615b45a0',0,0,NULL,'2025-10-17 13:38:39','2025-10-17 13:38:56',0,0),(23,'sonyloverxd','sony@lover','REDACTED_PASSWORD_HASH','1b2b04f9bc8725738e5dba3b438cb390',0,0,NULL,'2025-10-19 00:54:22','2025-10-19 00:54:57',0,0),(24,'jep','jeffj@eff','REDACTED_PASSWORD_HASH','053f8fe3eaa27d771de73400e3e287d2',0,0,NULL,'2025-10-19 00:58:46',NULL,0,0),(25,'edmar','edmar@edmar','REDACTED_PASSWORD_HASH','b604d066f78c2135e1d139634c878c61',0,0,NULL,'2025-10-22 19:49:03','2026-07-05 22:25:26',0,1),(26,'devjeffo','dev@jeffo','REDACTED_PASSWORD_HASH','7fe44b3d19c02a13fff9e53cd7d4266c',0,0,NULL,'2025-10-29 23:03:41','2025-10-30 00:43:05',0,0),(27,'shaolim','shaolim@matador','REDACTED_PASSWORD_HASH','7354ca4c6db8c68106559e026825985d',0,0,NULL,'2025-10-30 12:34:56','2026-02-09 01:14:24',0,0),(28,'jeffoso','jef@foso','REDACTED_PASSWORD_HASH','66c30416c36a682bcaf3cbb77fe37f86',0,0,NULL,'2025-11-01 02:08:42','2025-11-01 02:08:51',0,0),(29,'jeffu','jeffu@jeff','REDACTED_PASSWORD_HASH','21bf061affe90b1e91d2a3c64d6931e6',0,0,NULL,'2025-11-04 19:24:07','2026-07-05 18:35:33',12000,30),(30,'jeffus','jeffus@jeffo','REDACTED_PASSWORD_HASH','7c4e6cdae4965f0a23c2b1c5476d8077',0,0,NULL,'2025-12-04 21:29:37','2025-12-04 21:32:34',0,0),(31,'brunao','brunao@brunao','REDACTED_PASSWORD_HASH','4c7b872d727a46d07471b977730f6742',0,0,NULL,'2026-02-25 00:33:52','2026-02-25 00:34:33',0,0),(32,'jacktalbeck','jacktalbeck@junior','REDACTED_PASSWORD_HASH','34ecf86a846816e15457e2231c4df48d',0,0,NULL,'2026-04-19 07:12:20','2026-04-19 07:30:22',0,0),(33,'eljeforino','jeffo@roso.com','REDACTED_PASSWORD_HASH','ef3869edbbf223edac29611415aa81e4',0,0,NULL,'2026-04-28 02:17:34','2026-04-28 02:17:49',0,0),(34,'joao','joao@joao','REDACTED_PASSWORD_HASH','7e52be6c62f4a3c8be27ab6223faedeb',0,0,NULL,'2026-05-06 16:07:20','2026-05-10 21:09:27',0,0),(35,'wilis','wilis@wqilis','REDACTED_PASSWORD_HASH','d1640b4094153165c5a60180f01fc497',0,0,NULL,'2026-05-10 13:47:07','2026-05-10 13:47:50',0,0);
/*!40000 ALTER TABLE `accounts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `active_buffs`
--

DROP TABLE IF EXISTS `active_buffs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `active_buffs` (
  `buff_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `target_player_id` bigint unsigned NOT NULL,
  `source_player_id` bigint unsigned DEFAULT NULL,
  `skill_id` int unsigned NOT NULL,
  `effect_id` int unsigned DEFAULT NULL,
  `buff_type` enum('BUFF','DEBUFF','AURA','DOT','HOT','SHIELD') COLLATE utf8mb4_unicode_ci NOT NULL,
  `current_stacks` tinyint unsigned NOT NULL DEFAULT '1',
  `value_snapshot` int NOT NULL DEFAULT '0',
  `started_at` timestamp(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  `expires_at` timestamp(3) NOT NULL,
  `last_tick_at` timestamp(3) NULL DEFAULT NULL,
  `is_permanent` tinyint(1) NOT NULL DEFAULT '0',
  `snapshot_json` json DEFAULT NULL,
  PRIMARY KEY (`buff_id`),
  KEY `idx_target_buffs` (`target_player_id`,`expires_at`),
  KEY `idx_source_buffs` (`source_player_id`),
  KEY `idx_skill_buffs` (`skill_id`),
  KEY `idx_expires` (`expires_at`),
  CONSTRAINT `fk_buff_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_buff_source` FOREIGN KEY (`source_player_id`) REFERENCES `players` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_buff_target` FOREIGN KEY (`target_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=2002 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `active_buffs`
--

LOCK TABLES `active_buffs` WRITE;
/*!40000 ALTER TABLE `active_buffs` DISABLE KEYS */;
/*!40000 ALTER TABLE `active_buffs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `active_dots`
--

DROP TABLE IF EXISTS `active_dots`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `active_dots` (
  `dot_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `target_player_id` bigint unsigned NOT NULL,
  `source_player_id` bigint unsigned DEFAULT NULL,
  `skill_id` int unsigned NOT NULL,
  `effect_id` int unsigned DEFAULT NULL,
  `dot_type` enum('DAMAGE','HEAL','MANA') COLLATE utf8mb4_unicode_ci NOT NULL,
  `element_id` tinyint unsigned NOT NULL DEFAULT '1',
  `tick_value` int NOT NULL DEFAULT '0',
  `tick_interval_ms` int unsigned NOT NULL DEFAULT '1000',
  `ticks_remaining` tinyint unsigned NOT NULL DEFAULT '0',
  `started_at` timestamp(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  `next_tick_at` timestamp(3) NOT NULL,
  `expires_at` timestamp(3) NOT NULL,
  `snapshot_json` json DEFAULT NULL,
  PRIMARY KEY (`dot_id`),
  KEY `idx_target_dots` (`target_player_id`,`next_tick_at`),
  KEY `idx_next_tick` (`next_tick_at`),
  KEY `fk_dot_source` (`source_player_id`),
  KEY `fk_dot_skill` (`skill_id`),
  CONSTRAINT `fk_dot_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_dot_source` FOREIGN KEY (`source_player_id`) REFERENCES `players` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_dot_target` FOREIGN KEY (`target_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=52 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `active_dots`
--

LOCK TABLES `active_dots` WRITE;
/*!40000 ALTER TABLE `active_dots` DISABLE KEYS */;
/*!40000 ALTER TABLE `active_dots` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `auction_listings`
--

DROP TABLE IF EXISTS `auction_listings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `auction_listings` (
  `listing_id` int unsigned NOT NULL AUTO_INCREMENT,
  `seller_player_id` bigint unsigned NOT NULL,
  `inventory_id` int NOT NULL,
  `price_gold` bigint unsigned NOT NULL,
  `status` enum('active','sold','cancelled','expired') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'active',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `expires_at` timestamp NOT NULL,
  PRIMARY KEY (`listing_id`),
  KEY `idx_status_expires` (`status`,`expires_at`),
  KEY `idx_seller_status` (`seller_player_id`,`status`),
  KEY `idx_inv_status` (`inventory_id`,`status`),
  CONSTRAINT `auction_listings_ibfk_1` FOREIGN KEY (`seller_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `auction_listings_ibfk_2` FOREIGN KEY (`inventory_id`) REFERENCES `player_inventory` (`inventory_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=39 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `auction_listings`
--

LOCK TABLES `auction_listings` WRITE;
/*!40000 ALTER TABLE `auction_listings` DISABLE KEYS */;
INSERT INTO `auction_listings` VALUES (1,27,106,1000000,'cancelled','2026-04-14 02:44:18','2026-04-17 05:44:18'),(2,27,106,150,'sold','2026-04-14 02:45:40','2026-04-17 05:45:40'),(6,23,228,150,'sold','2026-04-14 16:03:58','2026-04-17 19:03:58'),(7,1,225,333,'sold','2026-04-14 17:34:37','2026-04-17 20:34:37'),(9,23,106,9999999,'expired','2026-04-15 13:32:40','2026-04-18 16:32:40'),(12,23,106,999999999,'expired','2026-04-27 20:03:16','2026-04-30 23:03:16'),(14,1,239,999999,'expired','2026-04-29 01:34:18','2026-05-02 04:34:18'),(15,1,94,50000,'sold','2026-05-06 16:26:23','2026-05-09 19:26:23'),(17,23,199,99999,'cancelled','2026-05-09 00:06:46','2026-05-12 03:06:46'),(22,1,242,999,'sold','2026-05-10 18:54:43','2026-05-13 21:54:43'),(23,23,91,1111,'sold','2026-05-10 19:50:56','2026-05-13 22:50:56'),(26,1,91,999,'sold','2026-05-13 01:16:41','2026-05-16 04:16:41'),(30,1,250,999,'expired','2026-05-17 03:21:24','2026-05-20 06:21:24'),(32,1,248,999,'expired','2026-05-17 03:21:35','2026-05-20 06:21:35'),(34,1,248,9999,'expired','2026-06-06 00:52:00','2026-06-09 03:52:00'),(37,1,96,1,'cancelled','2026-06-22 15:31:03','2026-06-25 18:31:03'),(38,32,296,99,'active','2026-07-01 17:35:09','2026-07-04 20:35:09');
/*!40000 ALTER TABLE `auction_listings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `basic_attacks`
--

DROP TABLE IF EXISTS `basic_attacks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `basic_attacks` (
  `class_id` bigint unsigned NOT NULL,
  `attack_name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `power_coef` smallint unsigned NOT NULL DEFAULT '80',
  `damage_type` enum('PHYSICAL','MAGIC','TRUE') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'PHYSICAL',
  `element_id` tinyint unsigned NOT NULL DEFAULT '1',
  `cooldown_ms` int unsigned NOT NULL DEFAULT '800',
  `range_max` smallint unsigned NOT NULL DEFAULT '250',
  `cast_anim_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `vfx_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `sfx_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `hit_window_ms` int unsigned NOT NULL DEFAULT '300',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`class_id`),
  KEY `fk_basic_attacks_element` (`element_id`),
  CONSTRAINT `fk_basic_attacks_class` FOREIGN KEY (`class_id`) REFERENCES `classes` (`class_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_basic_attacks_element` FOREIGN KEY (`element_id`) REFERENCES `skill_elements` (`element_id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `basic_attacks`
--

LOCK TABLES `basic_attacks` WRITE;
/*!40000 ALTER TABLE `basic_attacks` DISABLE KEYS */;
INSERT INTO `basic_attacks` VALUES (1,'Barbarian Basic Attack',85,'PHYSICAL',1,800,250,'/Game/Animations/Combat/AM_Barb_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Physical','/Game/SFX/Combat/SFX_BasicHit_Physical',300,'2026-06-01 04:01:46','2026-06-01 04:01:46'),(2,'Templar Basic Attack',82,'PHYSICAL',4,850,250,'/Game/Animations/Combat/AM_Temp_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Holy','/Game/SFX/Combat/SFX_BasicHit_Holy',320,'2026-06-01 04:01:46','2026-06-01 04:01:46'),(3,'DarkMage Basic Attack',78,'MAGIC',2,900,300,'/Game/Animations/Combat/AM_DMage_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Shadow','/Game/SFX/Combat/SFX_BasicHit_Shadow',350,'2026-06-01 04:01:46','2026-06-01 04:01:46'),(4,'Cleric Basic Attack',80,'MAGIC',4,900,280,'/Game/Animations/Combat/AM_Cleric_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Holy','/Game/SFX/Combat/SFX_BasicHit_Holy',340,'2026-06-01 04:01:46','2026-06-01 04:01:46'),(5,'Assassin Basic Attack',88,'PHYSICAL',1,700,220,'/Game/Animations/Combat/AM_Assn_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Crit','/Game/SFX/Combat/SFX_BasicHit_Crit',250,'2026-06-01 04:01:46','2026-06-01 04:01:46'),(6,'Monk Basic Attack',84,'PHYSICAL',1,750,240,'/Game/Animations/Combat/AM_Monk_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Wind','/Game/SFX/Combat/SFX_BasicHit_Wind',280,'2026-06-01 04:01:46','2026-06-01 04:01:46');
/*!40000 ALTER TABLE `basic_attacks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `blocked_players`
--

DROP TABLE IF EXISTS `blocked_players`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `blocked_players` (
  `block_id` int unsigned NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL COMMENT 'Jogador que bloqueou',
  `blocked_player_id` bigint unsigned NOT NULL COMMENT 'Jogador bloqueado',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`block_id`),
  UNIQUE KEY `unique_block` (`player_id`,`blocked_player_id`),
  KEY `idx_player` (`player_id`),
  KEY `idx_blocked` (`blocked_player_id`),
  CONSTRAINT `blocked_players_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `blocked_players_ibfk_2` FOREIGN KEY (`blocked_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `blocked_players`
--

LOCK TABLES `blocked_players` WRITE;
/*!40000 ALTER TABLE `blocked_players` DISABLE KEYS */;
/*!40000 ALTER TABLE `blocked_players` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `classes`
--

DROP TABLE IF EXISTS `classes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `classes` (
  `class_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `class_name` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `class_description` text COLLATE utf8mb4_unicode_ci,
  `base_strength` int unsigned DEFAULT '10',
  `base_dexterity` int unsigned DEFAULT '10',
  `base_intelligence` int unsigned DEFAULT '10',
  `base_vitality` int unsigned DEFAULT '10',
  `base_luck` int unsigned DEFAULT '10',
  `base_health` int unsigned DEFAULT '100',
  `base_mana` int unsigned DEFAULT '50',
  `base_stamina` int unsigned DEFAULT '100',
  `base_physical_attack` int DEFAULT '0',
  `base_magic_attack` int DEFAULT '0',
  `base_physical_defense` int DEFAULT '0',
  `base_magic_defense` int DEFAULT '0',
  `base_accuracy` int DEFAULT '0',
  `base_dodge` int DEFAULT '0',
  `base_critical` int DEFAULT '0',
  `base_movement` int DEFAULT '0',
  `base_critical_resistance` int DEFAULT '0',
  `base_double_attack_resistance` int DEFAULT '0',
  `base_double_attack_rate` int DEFAULT '0',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`class_id`),
  UNIQUE KEY `class_name` (`class_name`),
  KEY `idx_class_name` (`class_name`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `classes`
--

LOCK TABLES `classes` WRITE;
/*!40000 ALTER TABLE `classes` DISABLE KEYS */;
INSERT INTO `classes` VALUES (1,'Barbarian','Guerreiro selvagem focado em combate corpo a corpo e força bruta',20,12,8,18,10,150,30,120,15,2,12,5,8,6,10,5,3,0,5,'2025-11-27 16:19:42'),(2,'Templar','Cavaleiro sagrado equilibrado em ataque e defesa',15,10,12,20,12,180,60,110,10,8,15,10,10,8,8,4,8,0,3,'2025-11-27 16:19:42'),(3,'DarkMage','Mago das trevas especializado em magia ofensiva',8,10,22,10,12,80,150,90,3,20,5,15,12,5,15,3,12,0,2,'2025-11-27 16:19:42'),(4,'Cleric','Sacerdote especializado em cura e magia de suporte',10,10,18,15,15,120,140,100,5,15,8,12,10,7,8,4,15,0,2,'2025-11-27 16:19:42'),(5,'Assassin','Assassino furtivo especializado em ataques rápidos e críticos',12,22,10,10,18,100,40,130,12,5,6,4,18,20,25,8,2,0,15,'2025-11-27 16:19:42'),(6,'Monk','Monge equilibrado focado em velocidade e resistência',14,18,12,16,14,130,70,140,10,8,10,8,15,15,12,7,10,0,8,'2025-11-27 16:19:42');
/*!40000 ALTER TABLE `classes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `combat_log`
--

DROP TABLE IF EXISTS `combat_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `combat_log` (
  `log_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` timestamp(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  `source_player_id` bigint unsigned DEFAULT NULL,
  `target_player_id` bigint unsigned DEFAULT NULL,
  `skill_id` int unsigned DEFAULT NULL,
  `action_type` enum('DAMAGE','HEAL','BUFF','DEBUFF','DEATH','RESURRECT','MISS','DODGE','BLOCK','CRIT') COLLATE utf8mb4_unicode_ci NOT NULL,
  `value` int NOT NULL DEFAULT '0',
  `is_critical` tinyint(1) NOT NULL DEFAULT '0',
  `overkill` int NOT NULL DEFAULT '0',
  `zone_id` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `details_json` json DEFAULT NULL,
  PRIMARY KEY (`log_id`),
  KEY `idx_timestamp` (`timestamp`),
  KEY `idx_source` (`source_player_id`,`timestamp`),
  KEY `idx_target` (`target_player_id`,`timestamp`)
) ENGINE=InnoDB AUTO_INCREMENT=556 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `combat_log`
--

LOCK TABLES `combat_log` WRITE;
/*!40000 ALTER TABLE `combat_log` DISABLE KEYS */;
INSERT INTO `combat_log` VALUES (1,'2026-06-21 22:41:35.105',1,1,9,'HEAL',1390,0,0,'0',NULL),(2,'2026-06-21 22:43:09.351',1,1,9,'HEAL',1390,0,0,'0',NULL),(3,'2026-06-21 22:45:41.293',1,1,9,'HEAL',1390,0,0,'0',NULL),(4,'2026-06-21 22:46:30.294',1,1,9,'HEAL',1390,0,0,'0',NULL),(5,'2026-06-21 22:47:16.036',1,1,9,'HEAL',1390,0,0,'0',NULL),(6,'2026-06-21 22:51:58.907',1,1,9,'HEAL',1390,0,0,'0',NULL),(7,'2026-06-22 00:18:59.024',1,1,9,'HEAL',1390,0,0,'0',NULL),(8,'2026-06-22 00:19:59.896',1,1,9,'HEAL',1390,0,0,'0',NULL),(9,'2026-06-22 00:36:40.859',1,1,9,'HEAL',1390,0,0,'0',NULL),(10,'2026-06-22 00:42:36.330',1,1,9,'HEAL',1390,0,0,'0',NULL),(11,'2026-06-22 00:46:05.092',1,1,9,'HEAL',1390,0,0,'0',NULL),(12,'2026-06-22 00:48:01.280',1,1,9,'HEAL',1405,0,0,'0',NULL),(13,'2026-06-22 01:19:51.986',1,1,9,'HEAL',1390,0,0,'0',NULL),(14,'2026-06-22 01:42:26.373',1,1,9,'HEAL',1405,0,0,'0',NULL),(15,'2026-06-22 02:21:14.268',1,1,9,'HEAL',1405,0,0,'0',NULL),(16,'2026-06-22 02:33:56.234',1,1,9,'HEAL',1390,0,0,'0',NULL),(17,'2026-06-22 03:43:48.796',27,2,54,'HEAL',37,0,0,'0',NULL),(18,'2026-06-22 03:44:26.845',27,2,54,'HEAL',37,0,0,'0',NULL),(19,'2026-06-22 03:44:48.790',27,2,54,'HEAL',37,0,0,'0',NULL),(20,'2026-06-22 03:45:06.412',27,2,54,'HEAL',37,0,0,'0',NULL),(21,'2026-06-22 03:45:49.515',27,2,54,'HEAL',37,0,0,'0',NULL),(22,'2026-06-22 04:16:25.030',1,25,91,'DAMAGE',90,0,0,'0',NULL),(23,'2026-06-22 04:16:26.294',1,25,91,'DAMAGE',90,0,0,'0',NULL),(24,'2026-06-22 04:16:28.347',1,25,91,'DAMAGE',90,0,0,'0',NULL),(25,'2026-06-22 04:16:29.480',1,25,91,'DAMAGE',90,0,0,'0',NULL),(26,'2026-06-22 04:16:34.385',25,1,24,'DAMAGE',76,0,0,'0',NULL),(27,'2026-06-22 04:16:38.694',25,1,20,'DAMAGE',27,0,0,'0',NULL),(28,'2026-06-22 04:16:42.008',25,1,16,'DAMAGE',137,0,0,'0',NULL),(29,'2026-06-22 04:16:42.688',25,1,22,'DAMAGE',83,0,0,'0',NULL),(30,'2026-06-22 04:16:56.060',1,25,91,'DAMAGE',126,0,0,'0',NULL),(31,'2026-06-22 04:17:00.278',1,25,91,'DAMAGE',126,0,0,'0',NULL),(32,'2026-06-22 04:17:04.129',1,25,91,'DAMAGE',151,0,13,'0',NULL),(33,'2026-06-22 04:17:19.642',25,1,24,'DAMAGE',76,0,0,'0',NULL),(34,'2026-06-22 04:17:23.721',25,1,22,'DAMAGE',71,0,0,'0',NULL),(35,'2026-06-22 04:17:27.045',25,1,16,'DAMAGE',137,0,0,'0',NULL),(36,'2026-06-22 04:17:39.586',1,25,1,'DAMAGE',266,0,0,'0',NULL),(37,'2026-06-22 04:17:45.280',1,25,7,'DAMAGE',230,0,136,'0',NULL),(38,'2026-06-22 12:44:35.002',25,1,16,'DAMAGE',117,0,0,'0',NULL),(39,'2026-06-22 12:44:38.566',25,1,24,'DAMAGE',89,0,0,'0',NULL),(40,'2026-06-22 12:44:39.680',25,1,16,'DAMAGE',137,0,0,'0',NULL),(41,'2026-06-22 12:44:43.206',25,1,93,'DAMAGE',67,1,0,'0',NULL),(42,'2026-06-22 12:44:44.623',25,1,93,'DAMAGE',45,0,0,'0',NULL),(43,'2026-06-22 12:44:52.164',1,25,7,'DAMAGE',192,0,0,'0',NULL),(44,'2026-06-22 12:44:53.312',1,25,13,'DAMAGE',222,0,0,'0',NULL),(45,'2026-06-22 12:44:54.283',1,25,3,'DAMAGE',207,0,0,'0',NULL),(46,'2026-06-22 16:51:43.809',1,1,9,'HEAL',1148,0,0,'0',NULL),(47,'2026-06-24 02:22:37.799',25,1,93,'DAMAGE',10,0,0,'0',NULL),(48,'2026-06-24 02:23:12.277',25,1,93,'DAMAGE',10,0,0,'0',NULL),(49,'2026-06-24 03:38:43.080',25,1,20,'DAMAGE',27,0,0,'0',NULL),(50,'2026-06-24 03:40:32.558',1,25,13,'DAMAGE',228,0,0,'0',NULL),(51,'2026-06-24 03:40:52.285',25,1,20,'DAMAGE',27,0,0,'0',NULL),(52,'2026-06-24 03:41:44.403',25,1,20,'DAMAGE',27,0,0,'0',NULL),(53,'2026-06-24 03:42:24.504',1,25,13,'DAMAGE',228,0,0,'0',NULL),(54,'2026-06-24 03:43:38.042',25,1,20,'DAMAGE',27,0,0,'0',NULL),(55,'2026-06-24 03:44:09.637',1,25,13,'DAMAGE',222,0,0,'0',NULL),(56,'2026-06-24 03:44:42.779',1,25,13,'DAMAGE',228,0,0,'0',NULL),(57,'2026-06-24 03:45:04.087',25,1,20,'DAMAGE',27,0,0,'0',NULL),(58,'2026-06-24 03:45:09.475',25,1,24,'DAMAGE',76,0,0,'0',NULL),(59,'2026-06-24 04:36:29.117',1,25,13,'DAMAGE',222,0,0,'0',NULL),(60,'2026-06-24 04:37:07.660',1,25,13,'DAMAGE',222,0,0,'0',NULL),(61,'2026-06-24 04:37:28.498',1,25,1,'DAMAGE',266,0,0,'0',NULL),(62,'2026-06-24 04:37:33.090',25,1,24,'DAMAGE',76,0,0,'0',NULL),(63,'2026-06-24 13:47:32.595',1,1,9,'HEAL',325,0,0,'0',NULL),(64,'2026-06-24 14:28:33.104',1,25,13,'DAMAGE',222,0,0,'0',NULL),(65,'2026-06-24 14:28:59.818',1,25,13,'DAMAGE',222,0,0,'0',NULL),(66,'2026-06-24 14:31:16.205',1,25,13,'DAMAGE',223,0,0,'0',NULL),(67,'2026-06-24 14:31:29.692',25,1,16,'DAMAGE',117,0,0,'0',NULL),(68,'2026-06-24 15:38:17.799',25,1,93,'DAMAGE',38,0,0,'0',NULL),(69,'2026-06-24 15:38:18.771',25,1,93,'DAMAGE',38,0,0,'0',NULL),(70,'2026-06-24 15:38:20.653',25,1,93,'DAMAGE',38,0,0,'0',NULL),(71,'2026-06-24 18:20:33.106',23,1,76,'DAMAGE',47,1,0,'0',NULL),(72,'2026-06-24 18:22:13.709',1,23,13,'DAMAGE',277,0,0,'0',NULL),(73,'2026-06-24 18:23:14.255',1,23,13,'DAMAGE',277,0,0,'0',NULL),(74,'2026-06-25 00:51:44.126',25,1,20,'DAMAGE',27,0,0,'0',NULL),(75,'2026-06-25 02:41:21.530',1,25,13,'DAMAGE',228,0,0,'0',NULL),(76,'2026-06-25 03:10:18.459',1,25,13,'DAMAGE',222,0,0,'0',NULL),(77,'2026-06-25 03:13:54.015',1,25,13,'DAMAGE',222,0,0,'0',NULL),(78,'2026-06-25 03:20:11.135',1,25,13,'DAMAGE',228,0,0,'0',NULL),(79,'2026-06-25 04:01:29.123',1,25,13,'DAMAGE',222,0,0,'0',NULL),(80,'2026-06-25 04:01:50.033',1,25,13,'DAMAGE',266,0,0,'0',NULL),(81,'2026-06-25 04:38:44.004',1,25,13,'DAMAGE',222,0,0,'0',NULL),(82,'2026-06-25 05:11:37.375',1,25,13,'DAMAGE',310,0,0,'0',NULL),(83,'2026-06-25 05:13:33.765',1,25,13,'DAMAGE',222,0,0,'0',NULL),(84,'2026-06-25 05:14:14.165',1,25,13,'DAMAGE',266,0,0,'0',NULL),(85,'2026-06-25 13:19:02.056',1,25,13,'DAMAGE',222,0,0,'0',NULL),(86,'2026-06-25 13:19:55.464',1,25,13,'DAMAGE',222,0,0,'0',NULL),(87,'2026-06-25 13:20:11.773',1,25,13,'DAMAGE',222,0,0,'0',NULL),(88,'2026-06-25 14:28:33.829',1,25,13,'DAMAGE',222,0,0,'0',NULL),(89,'2026-06-25 14:30:17.519',1,25,13,'DAMAGE',222,0,0,'0',NULL),(90,'2026-06-25 14:31:32.258',1,25,13,'DAMAGE',222,0,0,'0',NULL),(91,'2026-06-25 14:32:26.454',1,25,13,'DAMAGE',222,0,0,'0',NULL),(92,'2026-06-25 14:32:45.624',1,25,13,'DAMAGE',222,0,0,'0',NULL),(93,'2026-06-25 14:32:52.577',1,25,3,'DAMAGE',207,0,0,'0',NULL),(94,'2026-06-25 14:33:19.910',25,1,24,'DAMAGE',76,0,0,'0',NULL),(95,'2026-06-25 14:33:38.710',25,1,16,'DAMAGE',137,0,0,'0',NULL),(96,'2026-06-25 15:29:32.171',32,1,31,'DAMAGE',10,0,0,'0',NULL),(97,'2026-06-27 13:56:50.502',32,1,42,'DAMAGE',128,0,0,'0',NULL),(98,'2026-06-27 13:56:55.535',32,1,37,'DAMAGE',113,0,0,'0',NULL),(99,'2026-06-27 13:56:59.637',32,1,31,'DAMAGE',145,0,0,'0',NULL),(100,'2026-06-27 13:57:04.087',32,1,33,'DAMAGE',92,0,0,'0',NULL),(101,'2026-06-27 13:57:05.501',32,1,31,'DAMAGE',128,0,0,'0',NULL),(102,'2026-06-27 13:57:06.328',32,1,96,'DAMAGE',60,0,0,'0',NULL),(103,'2026-06-27 13:57:15.273',32,1,37,'DAMAGE',113,0,0,'0',NULL),(104,'2026-06-27 13:58:33.610',1,32,13,'DAMAGE',166,0,0,'0',NULL),(105,'2026-06-27 15:08:56.944',1,32,91,'DAMAGE',67,0,0,'0',NULL),(106,'2026-06-27 15:08:59.643',1,32,7,'DAMAGE',125,0,0,'0',NULL),(107,'2026-06-27 15:09:00.185',1,32,3,'DAMAGE',135,0,0,'0',NULL),(108,'2026-06-27 15:09:01.747',1,32,91,'DAMAGE',58,0,0,'0',NULL),(109,'2026-06-27 15:09:05.554',32,1,96,'DAMAGE',60,0,0,'0',NULL),(110,'2026-06-27 15:09:06.206',1,32,1,'DAMAGE',173,0,0,'0',NULL),(111,'2026-06-27 15:09:06.310',32,1,31,'DAMAGE',128,0,0,'0',NULL),(112,'2026-06-27 15:09:29.296',32,1,96,'DAMAGE',60,0,0,'0',NULL),(113,'2026-06-27 15:09:30.149',32,1,31,'DAMAGE',128,0,0,'0',NULL),(114,'2026-06-27 15:09:32.478',32,1,33,'DAMAGE',92,0,0,'0',NULL),(115,'2026-06-27 15:09:36.324',32,32,39,'HEAL',232,0,0,'0',NULL),(116,'2026-06-27 15:09:38.914',32,1,42,'DAMAGE',128,0,0,'0',NULL),(117,'2026-06-27 15:09:41.273',32,1,31,'DAMAGE',166,0,0,'0',NULL),(118,'2026-06-27 15:09:43.955',32,1,96,'DAMAGE',77,0,0,'0',NULL),(119,'2026-06-27 15:09:44.605',32,1,31,'DAMAGE',166,0,0,'0',NULL),(120,'2026-06-27 15:09:45.484',32,1,33,'DAMAGE',120,0,0,'0',NULL),(121,'2026-06-27 15:09:48.799',32,1,96,'DAMAGE',77,0,0,'0',NULL),(122,'2026-06-27 15:09:49.588',32,1,37,'DAMAGE',147,0,0,'0',NULL),(123,'2026-06-27 15:09:53.953',1,32,91,'DAMAGE',50,0,0,'0',NULL),(124,'2026-06-27 15:09:53.956',32,1,96,'DAMAGE',88,0,0,'0',NULL),(125,'2026-06-27 15:09:54.407',1,32,1,'DAMAGE',148,0,0,'0',NULL),(126,'2026-06-27 15:09:54.411',32,1,31,'DAMAGE',189,0,0,'0',NULL),(127,'2026-06-27 15:09:55.612',1,32,13,'DAMAGE',124,0,0,'0',NULL),(128,'2026-06-27 15:10:18.973',1,1,9,'HEAL',325,0,0,'0',NULL),(129,'2026-06-27 15:10:22.001',32,32,39,'HEAL',232,0,0,'0',NULL),(130,'2026-06-27 15:10:40.456',1,32,7,'DAMAGE',144,0,0,'0',NULL),(131,'2026-06-27 15:10:43.643',1,32,13,'DAMAGE',166,0,0,'0',NULL),(132,'2026-06-27 15:10:45.286',1,32,1,'DAMAGE',200,0,0,'0',NULL),(133,'2026-06-27 15:10:48.574',32,1,96,'DAMAGE',60,0,0,'0',NULL),(134,'2026-06-27 15:10:49.430',32,1,31,'DAMAGE',128,0,0,'0',NULL),(135,'2026-06-27 15:10:52.251',32,1,96,'DAMAGE',60,0,0,'0',NULL),(136,'2026-06-27 15:10:54.110',32,1,33,'DAMAGE',92,0,0,'0',NULL),(137,'2026-06-27 15:10:55.189',32,1,37,'DAMAGE',113,0,0,'0',NULL),(138,'2026-06-27 15:10:57.008',32,1,31,'DAMAGE',145,0,0,'0',NULL),(139,'2026-06-27 18:38:00.692',1,1,9,'HEAL',653,0,0,'0',NULL),(140,'2026-06-27 22:22:38.339',1,32,91,'DAMAGE',67,0,0,'0',NULL),(141,'2026-06-27 22:22:38.864',1,32,1,'DAMAGE',200,0,0,'0',NULL),(142,'2026-06-27 22:22:39.533',1,32,13,'DAMAGE',166,0,0,'0',NULL),(143,'2026-06-27 22:22:41.931',1,32,3,'DAMAGE',155,0,0,'0',NULL),(144,'2026-06-27 22:22:42.951',1,32,91,'DAMAGE',67,0,0,'0',NULL),(145,'2026-06-27 22:22:46.334',1,32,1,'DAMAGE',200,0,0,'0',NULL),(146,'2026-06-27 22:22:49.966',32,1,31,'DAMAGE',128,0,0,'0',NULL),(147,'2026-06-27 22:22:51.924',32,1,37,'DAMAGE',113,0,0,'0',NULL),(148,'2026-06-27 22:22:54.615',32,1,42,'DAMAGE',145,0,0,'0',NULL),(149,'2026-06-27 22:22:57.178',32,1,33,'DAMAGE',136,0,0,'0',NULL),(150,'2026-06-27 22:22:58.222',32,32,39,'HEAL',232,0,0,'0',NULL),(151,'2026-06-27 22:23:01.507',32,1,96,'DAMAGE',77,0,0,'0',NULL),(152,'2026-06-27 22:23:01.511',1,32,91,'DAMAGE',58,0,0,'0',NULL),(153,'2026-06-27 22:23:03.497',32,1,96,'DAMAGE',77,0,0,'0',NULL),(154,'2026-06-27 22:23:03.782',32,1,33,'DAMAGE',120,0,0,'0',NULL),(155,'2026-06-27 22:23:09.777',32,1,96,'DAMAGE',77,0,0,'0',NULL),(156,'2026-06-27 22:23:10.300',32,1,31,'DAMAGE',166,0,0,'0',NULL),(157,'2026-06-27 22:23:11.782',32,1,33,'DAMAGE',120,0,0,'0',NULL),(158,'2026-06-27 22:23:11.785',1,32,7,'DAMAGE',107,0,0,'0',NULL),(159,'2026-06-27 22:23:13.392',32,1,37,'DAMAGE',221,1,146,'0',NULL),(160,'2026-06-27 22:24:26.742',1,32,91,'DAMAGE',67,0,0,'0',NULL),(161,'2026-06-27 22:24:27.394',1,32,1,'DAMAGE',200,0,0,'0',NULL),(162,'2026-06-27 22:24:28.508',1,32,7,'DAMAGE',125,0,0,'0',NULL),(163,'2026-06-27 22:24:29.121',1,32,3,'DAMAGE',135,0,0,'0',NULL),(164,'2026-06-27 22:24:30.144',1,32,91,'DAMAGE',87,1,0,'0',NULL),(165,'2026-06-27 22:24:33.966',1,32,1,'DAMAGE',173,0,0,'0',NULL),(166,'2026-06-27 23:42:09.282',32,1,96,'DAMAGE',60,0,0,'0',NULL),(167,'2026-06-27 23:42:09.286',1,32,91,'DAMAGE',67,0,0,'0',NULL),(168,'2026-06-27 23:42:11.875',32,1,96,'DAMAGE',60,0,0,'0',NULL),(169,'2026-06-27 23:42:13.232',1,32,91,'DAMAGE',67,0,0,'0',NULL),(170,'2026-06-27 23:42:14.072',32,1,96,'DAMAGE',60,0,0,'0',NULL),(171,'2026-06-27 23:42:14.715',1,32,91,'DAMAGE',67,0,0,'0',NULL),(172,'2026-06-27 23:42:15.286',32,1,96,'DAMAGE',60,0,0,'0',NULL),(173,'2026-06-27 23:42:15.928',1,32,91,'DAMAGE',67,0,0,'0',NULL),(174,'2026-06-27 23:42:16.530',32,1,96,'DAMAGE',60,0,0,'0',NULL),(175,'2026-06-27 23:42:17.141',1,32,91,'DAMAGE',67,0,0,'0',NULL),(176,'2026-06-27 23:42:17.578',32,1,96,'DAMAGE',60,0,0,'0',NULL),(177,'2026-06-27 23:42:18.457',1,32,91,'DAMAGE',67,0,0,'0',NULL),(178,'2026-06-27 23:42:19.667',32,1,96,'DAMAGE',60,0,0,'0',NULL),(179,'2026-06-27 23:42:19.670',1,32,91,'DAMAGE',67,0,0,'0',NULL),(180,'2026-06-27 23:42:20.778',32,1,96,'DAMAGE',60,0,0,'0',NULL),(181,'2026-06-27 23:42:21.826',1,32,91,'DAMAGE',67,0,0,'0',NULL),(182,'2026-06-27 23:42:21.830',32,1,96,'DAMAGE',60,0,0,'0',NULL),(183,'2026-06-27 23:42:22.972',32,1,96,'DAMAGE',60,0,0,'0',NULL),(184,'2026-06-27 23:42:22.976',1,32,91,'DAMAGE',67,0,0,'0',NULL),(185,'2026-06-27 23:42:24.051',32,1,96,'DAMAGE',60,0,0,'0',NULL),(186,'2026-06-27 23:42:24.054',1,32,91,'DAMAGE',67,0,0,'0',NULL),(187,'2026-06-27 23:42:24.993',32,1,96,'DAMAGE',60,0,0,'0',NULL),(188,'2026-06-27 23:42:25.969',32,1,96,'DAMAGE',60,0,0,'0',NULL),(189,'2026-06-27 23:42:26.977',32,1,96,'DAMAGE',60,0,0,'0',NULL),(190,'2026-06-27 23:42:27.022',1,32,91,'DAMAGE',67,0,0,'0',NULL),(191,'2026-06-27 23:42:27.929',1,32,91,'DAMAGE',67,0,0,'0',NULL),(192,'2026-06-27 23:42:28.835',32,1,96,'DAMAGE',60,0,0,'0',NULL),(193,'2026-06-27 23:42:29.745',32,1,96,'DAMAGE',60,0,0,'0',NULL),(194,'2026-06-27 23:42:29.789',1,32,91,'DAMAGE',67,0,0,'0',NULL),(195,'2026-06-27 23:42:30.625',32,1,96,'DAMAGE',60,0,0,'0',NULL),(196,'2026-06-27 23:42:31.534',32,1,96,'DAMAGE',60,0,0,'0',NULL),(197,'2026-06-27 23:42:32.515',32,1,96,'DAMAGE',60,0,0,'0',NULL),(198,'2026-06-27 23:42:32.955',1,32,91,'DAMAGE',67,0,0,'0',NULL),(199,'2026-06-27 23:42:33.387',32,1,96,'DAMAGE',60,0,0,'0',NULL),(200,'2026-06-27 23:43:05.541',1,32,91,'DAMAGE',67,0,0,'0',NULL),(201,'2026-06-27 23:43:07.805',1,32,91,'DAMAGE',67,0,0,'0',NULL),(202,'2026-06-27 23:43:10.162',1,32,91,'DAMAGE',67,0,0,'0',NULL),(203,'2026-06-27 23:43:11.179',1,32,91,'DAMAGE',67,0,0,'0',NULL),(204,'2026-06-27 23:43:12.226',1,32,91,'DAMAGE',67,0,0,'0',NULL),(205,'2026-06-27 23:43:16.238',1,32,91,'DAMAGE',67,0,0,'0',NULL),(206,'2026-06-27 23:43:17.216',1,32,91,'DAMAGE',67,0,0,'0',NULL),(207,'2026-06-27 23:43:18.193',1,32,91,'DAMAGE',67,0,0,'0',NULL),(208,'2026-06-27 23:43:30.854',1,32,13,'DAMAGE',166,0,0,'0',NULL),(209,'2026-06-27 23:44:24.943',1,32,13,'DAMAGE',166,0,0,'0',NULL),(210,'2026-06-27 23:49:53.685',1,32,91,'DAMAGE',67,0,0,'0',NULL),(211,'2026-06-27 23:49:55.022',1,32,91,'DAMAGE',67,0,0,'0',NULL),(212,'2026-06-27 23:49:55.489',1,32,1,'DAMAGE',200,0,0,'0',NULL),(213,'2026-06-27 23:50:09.777',1,32,91,'DAMAGE',67,0,0,'0',NULL),(214,'2026-06-27 23:50:09.780',32,1,96,'DAMAGE',60,0,0,'0',NULL),(215,'2026-06-27 23:50:10.637',32,1,31,'DAMAGE',128,0,0,'0',NULL),(216,'2026-06-27 23:50:11.140',1,32,13,'DAMAGE',166,0,0,'0',NULL),(217,'2026-06-27 23:50:11.648',32,1,33,'DAMAGE',92,0,0,'0',NULL),(218,'2026-06-27 23:50:11.650',1,32,7,'DAMAGE',125,0,0,'0',NULL),(219,'2026-06-27 23:50:12.087',1,32,3,'DAMAGE',135,0,0,'0',NULL),(220,'2026-06-27 23:50:12.599',32,1,37,'DAMAGE',113,0,0,'0',NULL),(221,'2026-06-27 23:50:13.018',1,32,91,'DAMAGE',58,0,0,'0',NULL),(222,'2026-06-27 23:50:13.020',32,1,96,'DAMAGE',68,0,0,'0',NULL),(223,'2026-06-27 23:50:13.989',1,32,1,'DAMAGE',173,0,0,'0',NULL),(224,'2026-06-27 23:50:14.073',32,1,31,'DAMAGE',145,0,0,'0',NULL),(225,'2026-06-27 23:50:16.292',1,32,91,'DAMAGE',58,0,0,'0',NULL),(226,'2026-06-27 23:50:16.361',32,1,96,'DAMAGE',68,0,0,'0',NULL),(227,'2026-06-27 23:50:17.631',1,32,91,'DAMAGE',58,0,0,'0',NULL),(228,'2026-06-27 23:50:17.635',32,1,96,'DAMAGE',68,0,0,'0',NULL),(229,'2026-06-27 23:50:17.882',1,32,1,'DAMAGE',173,0,124,'0',NULL),(230,'2026-06-27 23:50:18.421',1,32,7,'DAMAGE',125,0,125,'0',NULL),(231,'2026-06-27 23:57:03.458',32,1,96,'DAMAGE',70,0,0,'0',NULL),(232,'2026-06-27 23:57:05.523',32,1,33,'DAMAGE',108,0,0,'0',NULL),(233,'2026-06-27 23:57:06.492',1,32,3,'DAMAGE',155,0,0,'0',NULL),(234,'2026-06-27 23:57:09.049',1,32,1,'DAMAGE',200,0,0,'0',NULL),(235,'2026-06-27 23:57:09.069',32,1,31,'DAMAGE',150,0,0,'0',NULL),(236,'2026-06-27 23:57:09.098',1,32,91,'DAMAGE',67,0,0,'0',NULL),(237,'2026-06-27 23:57:09.109',32,1,96,'DAMAGE',70,0,0,'0',NULL),(238,'2026-06-27 23:57:09.176',32,1,37,'DAMAGE',133,0,0,'0',NULL),(239,'2026-06-27 23:57:09.202',1,32,13,'DAMAGE',166,0,0,'0',NULL),(240,'2026-06-27 23:57:09.987',1,32,91,'DAMAGE',58,0,0,'0',NULL),(241,'2026-06-27 23:57:09.999',32,1,96,'DAMAGE',80,0,0,'0',NULL),(242,'2026-06-27 23:57:11.239',32,1,96,'DAMAGE',80,0,0,'0',NULL),(243,'2026-06-27 23:57:11.320',1,32,91,'DAMAGE',58,0,0,'0',NULL),(244,'2026-06-27 23:57:12.410',32,1,33,'DAMAGE',123,0,0,'0',NULL),(245,'2026-06-27 23:57:12.490',1,32,7,'DAMAGE',125,0,0,'0',NULL),(246,'2026-06-27 23:57:14.049',32,1,31,'DAMAGE',170,0,0,'0',NULL),(247,'2026-06-27 23:57:14.135',1,32,1,'DAMAGE',173,0,0,'0',NULL),(248,'2026-06-27 23:57:14.760',1,32,91,'DAMAGE',58,0,0,'0',NULL),(249,'2026-06-27 23:57:14.763',32,1,96,'DAMAGE',80,0,0,'0',NULL),(250,'2026-06-27 23:57:16.218',1,32,3,'DAMAGE',135,0,0,'0',NULL),(251,'2026-06-27 23:57:16.627',32,1,96,'DAMAGE',70,0,0,'0',NULL),(252,'2026-06-27 23:57:17.190',1,32,91,'DAMAGE',58,0,0,'0',NULL),(253,'2026-06-27 23:57:17.708',32,1,31,'DAMAGE',150,0,0,'0',NULL),(254,'2026-06-27 23:57:18.358',1,32,1,'DAMAGE',200,0,161,'0',NULL),(255,'2026-06-27 23:57:18.631',32,1,96,'DAMAGE',70,0,0,'0',NULL),(256,'2026-06-27 23:57:18.885',1,32,91,'DAMAGE',67,0,67,'0',NULL),(257,'2026-06-28 00:00:07.469',32,1,96,'DAMAGE',104,0,0,'0',NULL),(258,'2026-06-28 00:00:08.444',32,1,96,'DAMAGE',104,0,0,'0',NULL),(259,'2026-06-28 00:00:08.449',1,32,91,'DAMAGE',98,0,0,'0',NULL),(260,'2026-06-28 00:00:09.385',1,32,91,'DAMAGE',98,0,0,'0',NULL),(261,'2026-06-28 00:00:09.389',32,1,96,'DAMAGE',104,0,0,'0',NULL),(262,'2026-06-28 00:00:10.324',32,1,96,'DAMAGE',104,0,0,'0',NULL),(263,'2026-06-28 00:00:11.854',32,1,31,'DAMAGE',221,0,0,'0',NULL),(264,'2026-06-28 00:00:13.000',32,1,33,'DAMAGE',160,0,0,'0',NULL),(265,'2026-06-28 00:00:13.947',1,32,3,'DAMAGE',193,0,0,'0',NULL),(266,'2026-06-28 00:00:16.099',1,32,1,'DAMAGE',248,0,0,'0',NULL),(267,'2026-06-28 00:00:16.102',32,1,31,'DAMAGE',221,0,0,'0',NULL),(268,'2026-06-28 00:00:17.654',1,32,91,'DAMAGE',84,0,0,'0',NULL),(269,'2026-06-28 00:00:17.661',32,1,96,'DAMAGE',104,0,66,'0',NULL),(270,'2026-06-28 00:00:38.905',1,1,9,'HEAL',325,0,0,'0',NULL),(271,'2026-06-28 00:00:44.804',32,32,39,'HEAL',280,0,0,'0',NULL),(272,'2026-06-28 00:01:16.108',32,1,42,'DAMAGE',170,0,0,'0',NULL),(273,'2026-06-28 00:01:20.212',32,1,96,'DAMAGE',80,0,0,'0',NULL),(274,'2026-06-28 00:01:20.216',1,32,91,'DAMAGE',81,0,0,'0',NULL),(275,'2026-06-28 00:01:21.173',1,32,1,'DAMAGE',239,0,0,'0',NULL),(276,'2026-06-28 00:01:21.609',1,32,13,'DAMAGE',199,0,0,'0',NULL),(277,'2026-06-28 00:01:22.249',1,32,7,'DAMAGE',150,0,0,'0',NULL),(278,'2026-06-28 00:01:22.921',1,32,3,'DAMAGE',161,0,0,'0',NULL),(279,'2026-06-28 00:01:23.579',1,32,91,'DAMAGE',70,0,0,'0',NULL),(280,'2026-06-28 00:01:23.582',32,1,96,'DAMAGE',80,0,0,'0',NULL),(281,'2026-06-28 00:01:24.271',1,32,1,'DAMAGE',207,0,0,'0',NULL),(282,'2026-06-28 00:01:24.274',32,1,31,'DAMAGE',170,0,0,'0',NULL),(283,'2026-06-28 00:01:24.591',1,32,91,'DAMAGE',70,0,0,'0',NULL),(284,'2026-06-28 00:01:24.632',32,1,96,'DAMAGE',80,0,0,'0',NULL),(285,'2026-06-28 00:02:12.864',1,1,9,'HEAL',325,0,0,'0',NULL),(286,'2026-06-28 00:02:14.040',32,32,39,'HEAL',280,0,0,'0',NULL),(287,'2026-06-28 00:03:18.031',32,1,42,'DAMAGE',150,0,0,'0',NULL),(288,'2026-06-28 00:03:21.680',32,1,96,'DAMAGE',104,0,0,'0',NULL),(289,'2026-06-28 00:03:22.721',32,1,96,'DAMAGE',104,0,0,'0',NULL),(290,'2026-06-28 00:03:24.981',32,1,96,'DAMAGE',104,0,0,'0',NULL),(291,'2026-06-28 00:03:24.984',1,32,91,'DAMAGE',98,0,0,'0',NULL),(292,'2026-06-28 00:03:27.115',1,32,3,'DAMAGE',225,0,0,'0',NULL),(293,'2026-06-28 00:03:28.459',32,1,31,'DAMAGE',221,0,0,'0',NULL),(294,'2026-06-28 00:03:29.655',32,1,96,'DAMAGE',104,0,0,'0',NULL),(295,'2026-06-28 00:03:29.694',1,32,91,'DAMAGE',98,0,0,'0',NULL),(296,'2026-06-28 00:03:31.755',32,1,31,'DAMAGE',221,0,7,'0',NULL),(297,'2026-06-28 00:04:02.865',1,32,91,'DAMAGE',67,0,0,'0',NULL),(298,'2026-06-28 00:04:17.749',32,1,31,'DAMAGE',150,0,0,'0',NULL),(299,'2026-06-28 00:04:17.753',1,32,1,'DAMAGE',200,0,0,'0',NULL),(300,'2026-06-28 00:04:21.023',1,32,13,'DAMAGE',166,0,0,'0',NULL),(301,'2026-06-28 00:04:22.198',1,32,7,'DAMAGE',125,0,0,'0',NULL),(302,'2026-06-28 00:04:22.202',32,1,33,'DAMAGE',108,0,0,'0',NULL),(303,'2026-06-28 00:04:23.235',1,32,3,'DAMAGE',135,0,0,'0',NULL),(304,'2026-06-28 00:04:24.234',32,1,37,'DAMAGE',133,0,0,'0',NULL),(305,'2026-06-28 00:07:12.724',32,1,96,'DAMAGE',70,0,0,'0',NULL),(306,'2026-06-28 00:07:12.727',1,32,91,'DAMAGE',67,0,0,'0',NULL),(307,'2026-06-28 00:07:13.633',1,32,91,'DAMAGE',67,0,0,'0',NULL),(308,'2026-06-28 00:07:29.852',32,32,39,'HEAL',280,0,0,'0',NULL),(309,'2026-06-28 00:08:08.416',32,1,96,'DAMAGE',70,0,0,'0',NULL),(310,'2026-06-28 00:08:08.420',1,32,91,'DAMAGE',67,0,0,'0',NULL),(311,'2026-06-28 00:08:09.223',32,1,96,'DAMAGE',70,0,0,'0',NULL),(312,'2026-06-28 00:08:10.228',32,1,96,'DAMAGE',70,0,0,'0',NULL),(313,'2026-06-28 00:08:10.368',1,32,91,'DAMAGE',67,0,0,'0',NULL),(314,'2026-06-28 00:08:11.170',32,1,96,'DAMAGE',105,1,0,'0',NULL),(315,'2026-06-28 00:08:12.004',32,1,96,'DAMAGE',70,0,0,'0',NULL),(316,'2026-06-28 00:08:12.849',32,1,96,'DAMAGE',70,0,0,'0',NULL),(317,'2026-06-28 00:08:13.188',1,32,91,'DAMAGE',67,0,0,'0',NULL),(318,'2026-06-28 00:08:13.661',32,1,96,'DAMAGE',70,0,0,'0',NULL),(319,'2026-06-28 00:08:14.496',32,1,96,'DAMAGE',70,0,0,'0',NULL),(320,'2026-06-28 00:08:15.305',32,1,96,'DAMAGE',70,0,0,'0',NULL),(321,'2026-06-28 00:08:16.145',1,32,91,'DAMAGE',67,0,0,'0',NULL),(322,'2026-06-28 00:08:16.185',32,1,96,'DAMAGE',70,0,0,'0',NULL),(323,'2026-06-28 00:08:17.972',32,1,96,'DAMAGE',70,0,0,'0',NULL),(324,'2026-06-28 00:08:18.878',32,1,96,'DAMAGE',70,0,0,'0',NULL),(325,'2026-06-28 00:08:19.796',1,32,91,'DAMAGE',67,0,0,'0',NULL),(326,'2026-06-28 00:08:19.799',32,1,96,'DAMAGE',70,0,0,'0',NULL),(327,'2026-06-28 00:08:20.772',1,32,91,'DAMAGE',67,0,0,'0',NULL),(328,'2026-06-28 00:08:20.775',32,1,96,'DAMAGE',70,0,0,'0',NULL),(329,'2026-06-28 00:08:21.806',32,1,96,'DAMAGE',70,0,0,'0',NULL),(330,'2026-06-28 00:08:21.878',1,32,91,'DAMAGE',67,0,0,'0',NULL),(331,'2026-06-28 00:08:23.662',32,1,96,'DAMAGE',70,0,0,'0',NULL),(332,'2026-06-28 00:08:23.811',1,32,91,'DAMAGE',67,0,0,'0',NULL),(333,'2026-06-28 00:08:24.772',32,1,96,'DAMAGE',70,0,0,'0',NULL),(334,'2026-06-28 00:08:25.789',1,32,91,'DAMAGE',67,0,0,'0',NULL),(335,'2026-06-28 00:08:25.792',32,1,96,'DAMAGE',70,0,0,'0',NULL),(336,'2026-06-28 00:08:26.696',32,1,96,'DAMAGE',70,0,20,'0',NULL),(337,'2026-06-28 00:08:26.700',1,32,91,'DAMAGE',67,0,0,'0',NULL),(338,'2026-06-28 00:09:05.544',1,1,9,'HEAL',325,0,0,'0',NULL),(339,'2026-06-28 00:10:20.964',1,32,91,'DAMAGE',70,0,0,'0',NULL),(340,'2026-06-28 00:10:20.971',32,1,96,'DAMAGE',45,0,0,'0',NULL),(341,'2026-06-28 00:10:21.869',1,32,91,'DAMAGE',70,0,0,'0',NULL),(342,'2026-06-28 00:10:21.873',32,1,96,'DAMAGE',45,0,0,'0',NULL),(343,'2026-06-28 00:10:22.851',32,1,96,'DAMAGE',45,0,0,'0',NULL),(344,'2026-06-28 00:10:22.891',1,32,91,'DAMAGE',70,0,0,'0',NULL),(345,'2026-06-28 00:10:23.700',32,1,96,'DAMAGE',45,0,0,'0',NULL),(346,'2026-06-28 00:10:23.832',1,32,91,'DAMAGE',70,0,0,'0',NULL),(347,'2026-06-28 00:10:25.448',32,1,96,'DAMAGE',45,0,0,'0',NULL),(348,'2026-06-28 00:10:26.793',1,32,91,'DAMAGE',70,0,0,'0',NULL),(349,'2026-06-28 00:10:27.266',32,1,96,'DAMAGE',45,0,0,'0',NULL),(350,'2026-06-28 00:10:27.771',1,32,91,'DAMAGE',70,0,0,'0',NULL),(351,'2026-06-28 00:10:28.174',32,1,96,'DAMAGE',45,0,0,'0',NULL),(352,'2026-06-28 00:10:29.157',32,1,96,'DAMAGE',45,0,0,'0',NULL),(353,'2026-06-28 00:10:29.631',1,32,91,'DAMAGE',70,0,0,'0',NULL),(354,'2026-06-28 00:10:30.066',32,1,96,'DAMAGE',45,0,0,'0',NULL),(355,'2026-06-28 00:10:30.573',1,32,91,'DAMAGE',70,0,0,'0',NULL),(356,'2026-06-28 00:10:31.041',32,1,96,'DAMAGE',45,0,0,'0',NULL),(357,'2026-06-28 00:10:31.512',1,32,91,'DAMAGE',70,0,0,'0',NULL),(358,'2026-06-28 00:10:31.954',32,1,96,'DAMAGE',45,0,0,'0',NULL),(359,'2026-06-28 00:10:32.426',1,32,91,'DAMAGE',70,0,0,'0',NULL),(360,'2026-06-28 00:10:33.060',32,1,96,'DAMAGE',45,0,0,'0',NULL),(361,'2026-06-28 00:10:33.359',1,32,91,'DAMAGE',70,0,0,'0',NULL),(362,'2026-06-28 00:10:34.061',32,1,96,'DAMAGE',45,0,0,'0',NULL),(363,'2026-06-28 00:10:34.366',1,32,91,'DAMAGE',70,0,0,'0',NULL),(364,'2026-06-28 00:10:35.005',32,1,96,'DAMAGE',45,0,0,'0',NULL),(365,'2026-06-28 00:10:35.313',1,32,91,'DAMAGE',70,0,0,'0',NULL),(366,'2026-06-28 00:10:35.951',32,1,96,'DAMAGE',45,0,0,'0',NULL),(367,'2026-06-28 00:10:37.001',32,1,96,'DAMAGE',45,0,0,'0',NULL),(368,'2026-06-28 00:10:37.193',1,32,91,'DAMAGE',70,0,0,'0',NULL),(369,'2026-06-28 00:10:39.223',1,32,91,'DAMAGE',70,0,0,'0',NULL),(370,'2026-06-28 00:10:40.227',1,32,91,'DAMAGE',70,0,0,'0',NULL),(371,'2026-06-28 00:10:40.347',32,32,39,'HEAL',280,0,0,'0',NULL),(372,'2026-06-28 00:10:41.603',32,1,96,'DAMAGE',45,0,0,'0',NULL),(373,'2026-06-28 00:10:42.310',1,32,91,'DAMAGE',70,0,0,'0',NULL),(374,'2026-06-28 00:10:43.290',32,1,96,'DAMAGE',45,0,0,'0',NULL),(375,'2026-06-28 00:10:43.295',1,32,91,'DAMAGE',70,0,0,'0',NULL),(376,'2026-06-28 00:10:44.265',32,1,96,'DAMAGE',45,0,0,'0',NULL),(377,'2026-06-28 00:10:44.269',1,32,91,'DAMAGE',70,0,0,'0',NULL),(378,'2026-06-28 00:10:45.139',32,1,96,'DAMAGE',45,0,0,'0',NULL),(379,'2026-06-28 00:10:45.980',32,1,96,'DAMAGE',45,0,0,'0',NULL),(380,'2026-06-28 00:10:46.958',32,1,96,'DAMAGE',45,0,0,'0',NULL),(381,'2026-06-28 00:10:47.127',1,32,91,'DAMAGE',70,0,0,'0',NULL),(382,'2026-06-28 00:10:47.928',32,1,96,'DAMAGE',45,0,0,'0',NULL),(383,'2026-06-28 00:10:48.852',32,1,96,'DAMAGE',45,0,0,'0',NULL),(384,'2026-06-28 00:10:49.059',1,32,91,'DAMAGE',70,0,0,'0',NULL),(385,'2026-06-28 00:10:49.687',32,1,96,'DAMAGE',45,0,0,'0',NULL),(386,'2026-06-28 00:10:50.025',1,32,91,'DAMAGE',70,0,0,'0',NULL),(387,'2026-06-28 00:10:51.002',1,32,91,'DAMAGE',70,0,14,'0',NULL),(388,'2026-06-28 00:11:07.862',1,1,9,'HEAL',358,0,0,'0',NULL),(389,'2026-06-28 06:32:37.571',32,32,39,'HEAL',280,0,0,'0',NULL),(390,'2026-07-01 04:22:42.118',1,1,9,'HEAL',1163,0,0,'0',NULL),(391,'2026-07-01 04:31:51.963',1,1,9,'HEAL',1398,0,0,'0',NULL),(392,'2026-07-01 05:06:02.906',32,1,96,'DAMAGE',81,0,0,'0',NULL),(393,'2026-07-01 05:06:18.833',32,32,39,'HEAL',945,0,0,'0',NULL),(394,'2026-07-01 05:06:58.638',32,32,39,'HEAL',945,0,0,'0',NULL),(395,'2026-07-01 05:07:11.282',32,1,96,'DAMAGE',62,0,0,'0',NULL),(396,'2026-07-01 05:07:11.890',32,1,31,'DAMAGE',133,0,0,'0',NULL),(397,'2026-07-01 05:07:14.023',32,1,33,'DAMAGE',96,0,0,'0',NULL),(398,'2026-07-01 05:07:15.633',32,1,96,'DAMAGE',62,0,0,'0',NULL),(399,'2026-07-01 05:07:15.879',32,1,31,'DAMAGE',133,0,0,'0',NULL),(400,'2026-07-01 05:07:17.418',1,1,9,'HEAL',1887,0,0,'0',NULL),(401,'2026-07-01 05:07:17.421',32,1,42,'DAMAGE',133,0,0,'0',NULL),(402,'2026-07-01 05:07:18.128',32,1,96,'DAMAGE',62,0,0,'0',NULL),(403,'2026-07-01 05:07:19.068',32,1,96,'DAMAGE',62,0,0,'0',NULL),(404,'2026-07-01 05:07:19.667',32,1,31,'DAMAGE',133,0,0,'0',NULL),(405,'2026-07-01 05:07:35.866',32,32,39,'HEAL',945,0,0,'0',NULL),(406,'2026-07-01 05:07:42.163',32,1,96,'DAMAGE',62,0,0,'0',NULL),(407,'2026-07-01 05:07:44.780',32,1,96,'DAMAGE',62,0,0,'0',NULL),(408,'2026-07-01 05:07:48.684',32,1,96,'DAMAGE',62,0,0,'0',NULL),(409,'2026-07-01 05:08:04.861',32,1,37,'DAMAGE',119,0,0,'0',NULL),(410,'2026-07-01 05:08:07.938',32,1,42,'DAMAGE',133,0,0,'0',NULL),(411,'2026-07-01 05:08:10.946',32,1,33,'DAMAGE',96,0,0,'0',NULL),(412,'2026-07-01 05:08:35.521',32,1,33,'DAMAGE',96,0,0,'0',NULL),(413,'2026-07-01 05:10:58.273',32,32,39,'HEAL',933,0,0,'0',NULL),(414,'2026-07-01 05:32:00.717',1,1,9,'HEAL',1872,0,0,'0',NULL),(415,'2026-07-01 05:32:01.639',32,32,39,'HEAL',1278,0,0,'0',NULL),(416,'2026-07-01 05:32:03.224',32,1,96,'DAMAGE',111,0,0,'0',NULL),(417,'2026-07-01 05:32:03.771',32,1,31,'DAMAGE',237,0,0,'0',NULL),(418,'2026-07-01 05:32:07.508',1,32,1,'DAMAGE',244,0,0,'0',NULL),(419,'2026-07-01 05:32:09.108',1,32,91,'DAMAGE',83,0,0,'0',NULL),(420,'2026-07-01 05:32:38.770',32,32,39,'HEAL',1278,0,0,'0',NULL),(421,'2026-07-01 05:32:58.731',1,1,9,'HEAL',1872,0,0,'0',NULL),(422,'2026-07-01 05:33:09.774',32,32,39,'HEAL',1278,0,0,'0',NULL),(423,'2026-07-01 05:33:24.273',32,1,96,'DAMAGE',85,0,0,'0',NULL),(424,'2026-07-01 05:33:25.564',32,1,96,'DAMAGE',85,0,0,'0',NULL),(425,'2026-07-01 05:33:26.017',32,1,33,'DAMAGE',132,0,0,'0',NULL),(426,'2026-07-01 05:33:27.086',32,1,96,'DAMAGE',85,0,0,'0',NULL),(427,'2026-07-01 05:33:27.946',32,1,31,'DAMAGE',182,0,0,'0',NULL),(428,'2026-07-01 05:33:29.005',32,1,37,'DAMAGE',162,0,0,'0',NULL),(429,'2026-07-01 05:33:31.583',32,1,31,'DAMAGE',182,0,0,'0',NULL),(430,'2026-07-01 05:33:42.591',32,32,39,'HEAL',1278,0,0,'0',NULL),(431,'2026-07-01 05:33:43.185',1,32,91,'DAMAGE',99,0,0,'0',NULL),(432,'2026-07-01 05:34:15.556',32,32,39,'HEAL',1278,0,0,'0',NULL),(433,'2026-07-01 05:35:08.913',1,1,9,'HEAL',1872,0,0,'0',NULL),(434,'2026-07-01 05:35:10.561',32,32,39,'HEAL',1278,0,0,'0',NULL),(435,'2026-07-01 05:35:47.181',32,32,39,'HEAL',1278,0,0,'0',NULL),(436,'2026-07-01 05:35:55.696',1,1,9,'HEAL',1872,0,0,'0',NULL),(437,'2026-07-01 05:37:15.584',1,32,1,'DAMAGE',246,0,0,'0',NULL),(438,'2026-07-01 05:37:19.355',32,1,31,'DAMAGE',183,0,0,'0',NULL),(439,'2026-07-01 05:37:25.809',1,32,91,'DAMAGE',83,0,0,'0',NULL),(440,'2026-07-01 05:37:26.911',1,1,9,'HEAL',1887,0,0,'0',NULL),(441,'2026-07-01 05:37:27.317',32,32,39,'HEAL',1290,0,0,'0',NULL),(442,'2026-07-01 05:37:27.637',1,32,91,'DAMAGE',83,0,0,'0',NULL),(443,'2026-07-01 05:37:28.070',32,1,96,'DAMAGE',86,0,0,'0',NULL),(444,'2026-07-01 05:37:28.957',32,1,33,'DAMAGE',132,0,0,'0',NULL),(445,'2026-07-01 05:37:34.684',1,32,91,'DAMAGE',83,0,0,'0',NULL),(446,'2026-07-01 05:37:35.399',1,32,1,'DAMAGE',246,0,0,'0',NULL),(447,'2026-07-01 05:37:36.005',32,1,96,'DAMAGE',86,0,0,'0',NULL),(448,'2026-07-01 05:37:36.958',1,32,3,'DAMAGE',191,0,0,'0',NULL),(449,'2026-07-01 05:38:14.782',32,32,39,'HEAL',1290,0,0,'0',NULL),(450,'2026-07-01 05:38:15.371',1,1,9,'HEAL',1887,0,0,'0',NULL),(451,'2026-07-01 05:38:47.728',32,32,39,'HEAL',1290,0,0,'0',NULL),(452,'2026-07-01 05:39:09.857',1,1,9,'HEAL',1887,0,0,'0',NULL),(453,'2026-07-01 05:40:16.950',32,32,39,'HEAL',1290,0,0,'0',NULL),(454,'2026-07-01 05:40:37.093',1,1,9,'HEAL',1887,0,0,'0',NULL),(455,'2026-07-01 05:40:48.106',1,32,91,'DAMAGE',100,0,0,'0',NULL),(456,'2026-07-01 05:40:48.451',32,32,39,'HEAL',1290,0,0,'0',NULL),(457,'2026-07-01 05:40:50.365',1,32,91,'DAMAGE',83,0,0,'0',NULL),(458,'2026-07-01 14:32:32.988',32,32,39,'HEAL',1278,0,0,'0',NULL),(459,'2026-07-01 14:33:21.156',32,32,39,'HEAL',1278,0,0,'0',NULL),(460,'2026-07-01 14:33:21.600',1,1,9,'HEAL',1872,0,0,'0',NULL),(461,'2026-07-01 14:33:37.875',32,1,96,'DAMAGE',85,0,43,'0',NULL),(462,'2026-07-01 14:33:38.089',32,1,31,'DAMAGE',182,0,182,'0',NULL),(463,'2026-07-01 14:34:11.234',1,1,9,'HEAL',1872,0,0,'0',NULL),(464,'2026-07-01 14:34:14.235',32,32,39,'HEAL',1278,0,0,'0',NULL),(465,'2026-07-01 14:34:44.928',32,32,39,'HEAL',1278,0,0,'0',NULL),(466,'2026-07-01 14:35:46.947',1,1,9,'HEAL',1872,0,0,'0',NULL),(467,'2026-07-01 14:35:51.810',32,32,39,'HEAL',1278,0,0,'0',NULL),(468,'2026-07-01 14:35:52.825',1,32,1,'DAMAGE',293,0,0,'0',NULL),(469,'2026-07-01 14:35:53.263',1,32,7,'DAMAGE',211,0,0,'0',NULL),(470,'2026-07-01 14:36:05.229',1,32,91,'DAMAGE',83,0,0,'0',NULL),(471,'2026-07-01 14:36:29.871',32,32,39,'HEAL',1278,0,0,'0',NULL),(472,'2026-07-01 14:37:02.548',32,32,39,'HEAL',1278,0,0,'0',NULL),(473,'2026-07-01 14:37:05.884',1,1,9,'HEAL',1872,0,0,'0',NULL),(474,'2026-07-02 18:17:02.867',32,32,39,'HEAL',1278,0,0,'0',NULL),(475,'2026-07-02 18:17:04.865',1,1,9,'HEAL',1872,0,0,'0',NULL),(476,'2026-07-02 18:17:04.941',32,1,42,'DAMAGE',237,0,0,'0',NULL),(477,'2026-07-02 18:17:05.403',32,1,96,'DAMAGE',111,0,0,'0',NULL),(478,'2026-07-02 18:17:06.051',32,1,31,'DAMAGE',237,0,0,'0',NULL),(479,'2026-07-02 18:17:12.666',32,1,96,'DAMAGE',85,0,0,'0',NULL),(480,'2026-07-02 18:17:13.207',32,1,31,'DAMAGE',182,0,0,'0',NULL),(481,'2026-07-02 18:17:17.556',1,32,91,'DAMAGE',83,0,0,'0',NULL),(482,'2026-07-02 18:17:18.456',1,32,1,'DAMAGE',244,0,0,'0',NULL),(483,'2026-07-02 18:18:10.473',32,32,39,'HEAL',1278,0,0,'0',NULL),(484,'2026-07-02 18:19:44.966',32,1,96,'DAMAGE',85,0,0,'0',NULL),(485,'2026-07-02 18:19:45.550',32,1,31,'DAMAGE',182,0,0,'0',NULL),(486,'2026-07-02 18:19:45.553',1,32,1,'DAMAGE',293,0,0,'0',NULL),(487,'2026-07-02 18:19:46.257',1,1,9,'HEAL',1872,0,0,'0',NULL),(488,'2026-07-02 18:19:48.161',1,32,91,'DAMAGE',83,0,0,'0',NULL),(489,'2026-07-02 18:19:49.526',1,32,7,'DAMAGE',176,0,0,'0',NULL),(490,'2026-07-02 18:19:50.740',32,1,96,'DAMAGE',85,0,0,'0',NULL),(491,'2026-07-02 18:19:51.158',32,1,31,'DAMAGE',182,0,0,'0',NULL),(492,'2026-07-02 18:19:53.478',32,32,39,'HEAL',1278,0,0,'0',NULL),(493,'2026-07-02 18:19:56.660',32,1,96,'DAMAGE',85,0,0,'0',NULL),(494,'2026-07-02 18:19:56.663',1,32,91,'DAMAGE',83,0,0,'0',NULL),(495,'2026-07-02 18:20:01.252',1,32,7,'DAMAGE',176,0,0,'0',NULL),(496,'2026-07-02 18:20:39.283',1,1,9,'HEAL',1872,0,0,'0',NULL),(497,'2026-07-02 18:20:41.358',32,32,39,'HEAL',1278,0,0,'0',NULL),(498,'2026-07-02 18:21:07.572',1,32,91,'DAMAGE',83,0,0,'0',NULL),(499,'2026-07-02 18:21:07.578',32,1,96,'DAMAGE',85,0,0,'0',NULL),(500,'2026-07-02 18:21:07.853',32,1,31,'DAMAGE',182,0,0,'0',NULL),(501,'2026-07-02 18:21:07.857',1,32,1,'DAMAGE',244,0,0,'0',NULL),(502,'2026-07-02 18:21:08.408',1,32,13,'DAMAGE',203,0,0,'0',NULL),(503,'2026-07-02 18:21:09.114',1,32,7,'DAMAGE',176,0,0,'0',NULL),(504,'2026-07-02 18:21:13.413',32,32,39,'HEAL',1278,0,0,'0',NULL),(505,'2026-07-02 18:21:14.880',1,32,91,'DAMAGE',83,0,0,'0',NULL),(506,'2026-07-02 18:21:16.023',1,32,91,'DAMAGE',83,0,0,'0',NULL),(507,'2026-07-02 18:21:16.853',1,32,7,'DAMAGE',176,0,0,'0',NULL),(508,'2026-07-02 18:21:17.388',1,32,1,'DAMAGE',244,0,0,'0',NULL),(509,'2026-07-02 18:21:17.391',32,1,31,'DAMAGE',182,0,0,'0',NULL),(510,'2026-07-02 18:21:17.877',1,32,91,'DAMAGE',83,0,0,'0',NULL),(511,'2026-07-02 18:21:19.405',1,32,3,'DAMAGE',190,0,0,'0',NULL),(512,'2026-07-02 18:21:20.138',1,32,91,'DAMAGE',83,0,0,'0',NULL),(513,'2026-07-02 18:21:21.149',1,32,91,'DAMAGE',83,0,0,'0',NULL),(514,'2026-07-02 18:21:22.734',32,1,96,'DAMAGE',85,0,0,'0',NULL),(515,'2026-07-02 18:21:22.737',1,32,91,'DAMAGE',83,0,0,'0',NULL),(516,'2026-07-02 18:21:23.185',1,32,1,'DAMAGE',244,0,0,'0',NULL),(517,'2026-07-02 18:21:23.188',32,1,31,'DAMAGE',182,0,0,'0',NULL),(518,'2026-07-02 18:21:23.355',1,32,13,'DAMAGE',203,0,0,'0',NULL),(519,'2026-07-02 18:21:25.544',1,1,9,'HEAL',1872,0,0,'0',NULL),(520,'2026-07-02 18:21:31.043',1,32,91,'DAMAGE',83,0,78,'0',NULL),(521,'2026-07-02 18:24:25.108',1,32,91,'DAMAGE',83,0,0,'0',NULL),(522,'2026-07-02 18:24:25.664',1,32,1,'DAMAGE',244,0,0,'0',NULL),(523,'2026-07-02 18:24:27.183',1,32,7,'DAMAGE',176,0,0,'0',NULL),(524,'2026-07-02 18:24:27.483',1,32,3,'DAMAGE',190,0,0,'0',NULL),(525,'2026-07-02 18:24:29.131',32,32,39,'HEAL',1290,0,0,'0',NULL),(526,'2026-07-02 18:24:29.351',1,32,91,'DAMAGE',83,0,0,'0',NULL),(527,'2026-07-02 18:24:32.757',1,32,91,'DAMAGE',83,0,0,'0',NULL),(528,'2026-07-02 18:24:34.462',1,32,7,'DAMAGE',176,0,0,'0',NULL),(529,'2026-07-02 18:24:34.921',1,32,91,'DAMAGE',83,0,0,'0',NULL),(530,'2026-07-02 18:24:36.129',1,32,91,'DAMAGE',83,0,0,'0',NULL),(531,'2026-07-02 18:24:40.338',1,32,91,'DAMAGE',83,0,0,'0',NULL),(532,'2026-07-02 18:24:40.798',1,32,1,'DAMAGE',244,0,0,'0',NULL),(533,'2026-07-02 18:24:41.097',1,32,13,'DAMAGE',203,0,0,'0',NULL),(534,'2026-07-02 18:24:41.326',1,32,91,'DAMAGE',83,0,0,'0',NULL),(535,'2026-07-02 18:25:00.159',32,32,39,'HEAL',1290,0,0,'0',NULL),(536,'2026-07-02 18:26:01.772',1,1,9,'HEAL',1872,0,0,'0',NULL),(537,'2026-07-02 18:27:09.794',32,32,39,'HEAL',1290,0,0,'0',NULL),(538,'2026-07-02 18:27:10.590',1,1,9,'HEAL',1872,0,0,'0',NULL),(539,'2026-07-02 18:27:10.883',1,32,91,'DAMAGE',99,0,0,'0',NULL),(540,'2026-07-02 18:27:11.951',1,32,7,'DAMAGE',211,0,0,'0',NULL),(541,'2026-07-02 18:27:18.616',1,32,91,'DAMAGE',83,0,0,'0',NULL),(542,'2026-07-03 01:52:19.441',1,1,9,'HEAL',1872,0,0,'0',NULL),(543,'2026-07-04 00:40:50.394',62,4,61,'HEAL',40,0,0,'0',NULL),(544,'2026-07-04 00:41:19.599',62,4,61,'HEAL',40,0,0,'0',NULL),(545,'2026-07-04 00:41:27.403',62,4,61,'HEAL',40,0,0,'0',NULL),(546,'2026-07-04 01:13:29.865',32,32,39,'HEAL',1278,0,0,'0',NULL),(547,'2026-07-04 01:14:28.322',1,32,1,'DAMAGE',81,0,0,'0',NULL),(548,'2026-07-04 01:15:12.454',1,1,9,'HEAL',1872,0,0,'0',NULL),(549,'2026-07-04 01:16:18.022',32,32,39,'HEAL',1278,0,0,'0',NULL),(550,'2026-07-04 01:16:19.145',1,1,9,'HEAL',1872,0,0,'0',NULL),(551,'2026-07-04 01:16:25.945',1,32,91,'DAMAGE',99,0,0,'0',NULL),(552,'2026-07-04 01:16:32.910',1,32,91,'DAMAGE',99,0,0,'0',NULL),(553,'2026-07-04 01:16:33.465',1,32,1,'DAMAGE',293,0,0,'0',NULL),(554,'2026-07-04 01:16:33.503',32,1,31,'DAMAGE',183,0,0,'0',NULL),(555,'2026-07-04 01:17:33.592',32,32,39,'HEAL',1278,0,0,'0',NULL);
/*!40000 ALTER TABLE `combat_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `duel_requests`
--

DROP TABLE IF EXISTS `duel_requests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `duel_requests` (
  `request_id` int unsigned NOT NULL AUTO_INCREMENT,
  `challenger_id` bigint unsigned NOT NULL COMMENT 'Desafiante',
  `opponent_id` bigint unsigned NOT NULL COMMENT 'Oponente',
  `status` enum('pending','accepted','declined','expired','cancelled') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `responded_at` timestamp NULL DEFAULT NULL,
  `expires_at` timestamp NULL DEFAULT NULL COMMENT 'Desafio expira após 1 minuto',
  PRIMARY KEY (`request_id`),
  KEY `idx_challenger` (`challenger_id`),
  KEY `idx_opponent` (`opponent_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `duel_requests_ibfk_1` FOREIGN KEY (`challenger_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `duel_requests_ibfk_2` FOREIGN KEY (`opponent_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `duel_requests`
--

LOCK TABLES `duel_requests` WRITE;
/*!40000 ALTER TABLE `duel_requests` DISABLE KEYS */;
/*!40000 ALTER TABLE `duel_requests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `exp_zones`
--

DROP TABLE IF EXISTS `exp_zones`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `exp_zones` (
  `exp_zone_id` int unsigned NOT NULL AUTO_INCREMENT,
  `zone_id` int unsigned NOT NULL COMMENT 'ID da zona C++ (zone_0 = 0)',
  `name` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'exp_zone',
  `center_x` float NOT NULL DEFAULT '0',
  `center_y` float NOT NULL DEFAULT '0',
  `center_z` float NOT NULL DEFAULT '0',
  `radius` float NOT NULL DEFAULT '1000' COMMENT 'Raio horizontal (cilindro)',
  `exp_per_tick` int NOT NULL DEFAULT '50',
  `tick_interval_sec` float NOT NULL DEFAULT '5',
  `min_player_level` int NOT NULL DEFAULT '0' COMMENT '0 = sem mínimo',
  `max_player_level` int NOT NULL DEFAULT '0' COMMENT '0 = sem máximo',
  `enabled` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`exp_zone_id`),
  KEY `idx_zone_enabled` (`zone_id`,`enabled`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `exp_zones`
--

LOCK TABLES `exp_zones` WRITE;
/*!40000 ALTER TABLE `exp_zones` DISABLE KEYS */;
INSERT INTO `exp_zones` VALUES (1,0,'training_area',-52,-2088,60,800,500,3,0,0,1,'2026-06-22 15:46:55','2026-06-22 17:11:14');
/*!40000 ALTER TABLE `exp_zones` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `factions`
--

DROP TABLE IF EXISTS `factions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `factions` (
  `faction_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `faction_name` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `faction_description` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`faction_id`),
  UNIQUE KEY `faction_name` (`faction_name`),
  KEY `idx_faction_name` (`faction_name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `factions`
--

LOCK TABLES `factions` WRITE;
/*!40000 ALTER TABLE `factions` DISABLE KEYS */;
INSERT INTO `factions` VALUES (1,'Novarra','Reino de luz e ordem, dedicado à justiça e proteção dos inocentes','2025-11-27 16:19:42'),(2,'Eldros','Império das sombras, focado em poder e dominação através da força','2025-11-27 16:19:42');
/*!40000 ALTER TABLE `factions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `friend_requests`
--

DROP TABLE IF EXISTS `friend_requests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `friend_requests` (
  `request_id` int unsigned NOT NULL AUTO_INCREMENT,
  `from_player_id` bigint unsigned NOT NULL,
  `to_player_id` bigint unsigned NOT NULL,
  `status` enum('pending','accepted','declined','expired') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `responded_at` timestamp NULL DEFAULT NULL,
  `expires_at` timestamp NULL DEFAULT NULL COMMENT 'Solicitação expira após 7 dias',
  PRIMARY KEY (`request_id`),
  UNIQUE KEY `unique_friend_request` (`from_player_id`,`to_player_id`),
  KEY `idx_from_player` (`from_player_id`),
  KEY `idx_to_player` (`to_player_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `friend_requests_ibfk_1` FOREIGN KEY (`from_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `friend_requests_ibfk_2` FOREIGN KEY (`to_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=31 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `friend_requests`
--

LOCK TABLES `friend_requests` WRITE;
/*!40000 ALTER TABLE `friend_requests` DISABLE KEYS */;
INSERT INTO `friend_requests` VALUES (1,1,23,'accepted','2026-02-11 02:36:44','2026-02-12 14:04:51','2026-02-19 17:04:49'),(2,4,23,'accepted','2026-02-11 03:10:21','2026-02-11 03:10:24','2026-02-18 06:10:21'),(3,2,14,'accepted','2026-02-11 03:36:10','2026-02-11 03:36:13','2026-02-18 06:36:10'),(6,23,1,'accepted','2026-02-11 23:48:56','2026-02-26 16:45:58','2026-03-05 19:45:57'),(17,14,25,'accepted','2026-02-13 02:28:50','2026-02-13 02:28:52','2026-02-20 05:28:50'),(20,32,1,'accepted','2026-02-24 15:11:18','2026-02-24 15:11:20','2026-03-03 18:11:18'),(21,34,1,'accepted','2026-02-25 00:36:26','2026-02-25 00:36:28','2026-03-04 03:36:26'),(24,1,25,'accepted','2026-03-06 01:18:22','2026-03-06 01:18:23','2026-03-13 04:18:22'),(25,25,23,'accepted','2026-03-15 08:18:44','2026-04-09 20:23:53','2026-04-16 20:23:52'),(26,23,25,'pending','2026-03-15 16:24:29',NULL,'2026-03-22 19:24:29'),(27,33,25,'accepted','2026-03-16 13:35:29','2026-03-16 13:35:38','2026-03-23 16:35:29'),(28,1,46,'accepted','2026-05-06 16:10:16','2026-05-06 16:10:18','2026-05-13 16:10:16'),(29,23,2,'accepted','2026-05-09 00:04:11','2026-05-09 00:04:13','2026-05-16 00:04:11'),(30,27,23,'accepted','2026-05-11 01:18:43','2026-05-11 01:18:46','2026-05-18 01:18:43');
/*!40000 ALTER TABLE `friend_requests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `friends`
--

DROP TABLE IF EXISTS `friends`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `friends` (
  `friendship_id` int unsigned NOT NULL AUTO_INCREMENT,
  `player1_id` bigint unsigned NOT NULL COMMENT 'Jogador 1 (menor ID)',
  `player2_id` bigint unsigned NOT NULL COMMENT 'Jogador 2 (maior ID)',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_interaction` timestamp NULL DEFAULT NULL COMMENT 'Última vez que interagiram',
  PRIMARY KEY (`friendship_id`),
  UNIQUE KEY `unique_friendship` (`player1_id`,`player2_id`),
  KEY `idx_player1` (`player1_id`),
  KEY `idx_player2` (`player2_id`),
  CONSTRAINT `friends_ibfk_1` FOREIGN KEY (`player1_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `friends_ibfk_2` FOREIGN KEY (`player2_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `friends_chk_1` CHECK ((`player1_id` < `player2_id`))
) ENGINE=InnoDB AUTO_INCREMENT=46 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `friends`
--

LOCK TABLES `friends` WRITE;
/*!40000 ALTER TABLE `friends` DISABLE KEYS */;
INSERT INTO `friends` VALUES (3,2,14,'2026-02-11 03:36:13',NULL),(31,14,25,'2026-02-13 02:28:52',NULL),(38,1,34,'2026-02-25 00:36:28',NULL),(39,1,23,'2026-02-26 16:45:58',NULL),(40,1,25,'2026-03-06 01:18:23',NULL),(41,25,33,'2026-03-16 13:35:38',NULL),(42,23,25,'2026-04-09 20:23:53',NULL),(43,1,46,'2026-05-06 16:10:18',NULL),(44,2,23,'2026-05-09 00:04:13',NULL),(45,23,27,'2026-05-11 01:18:46',NULL);
/*!40000 ALTER TABLE `friends` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `guild_invites`
--

DROP TABLE IF EXISTS `guild_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_invites` (
  `invite_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `guild_id` bigint unsigned NOT NULL,
  `invited_by_player_id` bigint unsigned NOT NULL,
  `invited_player_id` bigint unsigned NOT NULL,
  `status` enum('pending','accepted','declined','expired','cancelled') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `responded_at` timestamp NULL DEFAULT NULL,
  `expires_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`invite_id`),
  KEY `idx_guild_invited_player` (`invited_player_id`),
  KEY `idx_guild_invite_status` (`status`),
  KEY `guild_id` (`guild_id`),
  KEY `invited_by_player_id` (`invited_by_player_id`),
  CONSTRAINT `guild_invites_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`guild_id`) ON DELETE CASCADE,
  CONSTRAINT `guild_invites_ibfk_2` FOREIGN KEY (`invited_by_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `guild_invites_ibfk_3` FOREIGN KEY (`invited_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `guild_invites`
--

LOCK TABLES `guild_invites` WRITE;
/*!40000 ALTER TABLE `guild_invites` DISABLE KEYS */;
INSERT INTO `guild_invites` VALUES (2,4,23,25,'accepted','2026-04-17 16:47:35','2026-04-17 16:47:38','2026-04-20 16:47:35'),(3,4,23,2,'accepted','2026-04-27 20:05:49','2026-04-27 20:05:50','2026-04-30 20:05:49'),(5,4,23,27,'accepted','2026-05-16 00:42:07','2026-05-16 00:42:09','2026-05-19 00:42:07');
/*!40000 ALTER TABLE `guild_invites` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `bi_guild_invites_no_dup_pending` BEFORE INSERT ON `guild_invites` FOR EACH ROW BEGIN
  IF NEW.status = 'pending' AND EXISTS (
    SELECT 1
    FROM guild_invites gi
    WHERE gi.guild_id = NEW.guild_id
      AND gi.invited_player_id = NEW.invited_player_id
      AND gi.status = 'pending'
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Ja existe convite pendente para este jogador nesta guild.';
  END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `bu_guild_invites_no_dup_pending` BEFORE UPDATE ON `guild_invites` FOR EACH ROW BEGIN
  IF NEW.status = 'pending' AND EXISTS (
    SELECT 1
    FROM guild_invites gi
    WHERE gi.guild_id = NEW.guild_id
      AND gi.invited_player_id = NEW.invited_player_id
      AND gi.status = 'pending'
      AND gi.invite_id <> OLD.invite_id
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Ja existe convite pendente para este jogador nesta guild.';
  END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `guild_members`
--

DROP TABLE IF EXISTS `guild_members`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guild_members` (
  `guild_id` bigint unsigned NOT NULL,
  `player_id` bigint unsigned NOT NULL,
  `member_rank` tinyint unsigned NOT NULL DEFAULT '1' COMMENT '1=Comum, 2=Convida, 3=Convida+Remove',
  `contribution_xp` bigint unsigned NOT NULL DEFAULT '0',
  `joined_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`,`player_id`),
  UNIQUE KEY `unique_player_guild` (`player_id`),
  KEY `idx_guild_rank` (`guild_id`,`member_rank`),
  CONSTRAINT `guild_members_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`guild_id`) ON DELETE CASCADE,
  CONSTRAINT `guild_members_ibfk_2` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `guild_members`
--

LOCK TABLES `guild_members` WRITE;
/*!40000 ALTER TABLE `guild_members` DISABLE KEYS */;
INSERT INTO `guild_members` VALUES (4,2,2,0,'2026-04-27 20:05:50','2026-05-10 17:38:54'),(4,23,3,0,'2026-04-17 02:32:56','2026-04-17 02:32:56'),(4,25,1,0,'2026-04-17 16:47:38','2026-04-19 06:49:21'),(4,27,1,0,'2026-05-16 00:42:09','2026-05-16 00:42:09'),(6,1,3,0,'2026-05-10 17:28:15','2026-05-10 17:28:15');
/*!40000 ALTER TABLE `guild_members` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `guilds`
--

DROP TABLE IF EXISTS `guilds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `guilds` (
  `guild_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `guild_name` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `guild_tag` varchar(5) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `guild_description` text COLLATE utf8mb4_unicode_ci,
  `guild_icon` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `creation_cost_gold` bigint unsigned NOT NULL DEFAULT '0',
  `owner_player_id` bigint unsigned DEFAULT NULL,
  `guild_leader_id` bigint unsigned DEFAULT NULL,
  `guild_xp` bigint unsigned NOT NULL DEFAULT '0',
  `guild_level` int unsigned NOT NULL DEFAULT '1',
  `ranking_score` bigint unsigned NOT NULL DEFAULT '0',
  `member_limit` smallint unsigned NOT NULL DEFAULT '128',
  `member_count` smallint unsigned NOT NULL DEFAULT '1',
  `founded_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`guild_id`),
  UNIQUE KEY `guild_name` (`guild_name`),
  UNIQUE KEY `ux_guild_tag` (`guild_tag`),
  KEY `idx_guild_leader` (`guild_leader_id`),
  KEY `idx_guild_tag` (`guild_tag`),
  KEY `idx_guild_owner` (`owner_player_id`),
  KEY `idx_guild_ranking` (`ranking_score`),
  KEY `idx_guild_xp` (`guild_xp`),
  CONSTRAINT `fk_guild_owner_player` FOREIGN KEY (`owner_player_id`) REFERENCES `players` (`id`) ON DELETE SET NULL,
  CONSTRAINT `guilds_ibfk_1` FOREIGN KEY (`guild_leader_id`) REFERENCES `players` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `guilds`
--

LOCK TABLES `guilds` WRITE;
/*!40000 ALTER TABLE `guilds` DISABLE KEYS */;
INSERT INTO `guilds` VALUES (4,'Galaxy','PVP','GUILD',NULL,100000,23,23,0,1,0,128,4,'2026-04-17 02:32:56','2026-04-17 02:32:56','2026-05-16 00:42:09'),(6,'UniVersE','PVPVE','guild pvp pve para diversão',NULL,100000,1,1,0,1,0,128,1,'2026-05-10 17:28:15','2026-05-10 17:28:15','2026-05-10 17:28:15');
/*!40000 ALTER TABLE `guilds` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `bi_guilds_unique_tag` BEFORE INSERT ON `guilds` FOR EACH ROW BEGIN
  IF NEW.guild_tag IS NOT NULL AND TRIM(NEW.guild_tag) <> '' AND EXISTS (
    SELECT 1
    FROM guilds g
    WHERE UPPER(TRIM(g.guild_tag)) = UPPER(TRIM(NEW.guild_tag))
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'guild_tag ja esta em uso.';
  END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `bu_guilds_unique_tag` BEFORE UPDATE ON `guilds` FOR EACH ROW BEGIN
  IF NEW.guild_tag IS NOT NULL AND TRIM(NEW.guild_tag) <> '' AND EXISTS (
    SELECT 1
    FROM guilds g
    WHERE UPPER(TRIM(g.guild_tag)) = UPPER(TRIM(NEW.guild_tag))
      AND g.guild_id <> OLD.guild_id
  ) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'guild_tag ja esta em uso.';
  END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `item_templates`
--

DROP TABLE IF EXISTS `item_templates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `item_templates` (
  `item_id` int NOT NULL AUTO_INCREMENT,
  `item_name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `item_description` text COLLATE utf8mb4_unicode_ci,
  `item_type` enum('weapon','armor','consumable','material','quest','misc') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'misc',
  `item_subtype` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT 'Ex: sword, helmet, health_potion, ore, etc.',
  `icon_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT 'Caminho para o ícone do item no Content Browser',
  `max_stack_size` int NOT NULL DEFAULT '1' COMMENT 'Quantidade máxima em um slot (1 = não empilhável)',
  `equipment_slot` enum('none','head','chest','legs','feet','hands','main_hand','off_hand','ring','amulet','necklace','earring','bracelet','mount') COLLATE utf8mb4_unicode_ci DEFAULT 'none' COMMENT 'Slot de equipamento',
  `required_level` int DEFAULT '1' COMMENT 'Nível mínimo para usar o item',
  `stats_json` json DEFAULT NULL COMMENT 'Estatísticas do item: {"damage": 10, "defense": 5, "health_restore": 50, etc.}',
  `use_cooldown_ms` int unsigned NOT NULL DEFAULT '5000' COMMENT 'Cooldown em ms ao usar (por subtype)',
  `rarity` enum('common','uncommon','rare','epic','legendary') COLLATE utf8mb4_unicode_ci DEFAULT 'common',
  `value` int DEFAULT '0' COMMENT 'Valor em moeda do jogo para venda/compra',
  `weight` float DEFAULT '0' COMMENT 'Peso do item (para sistema de carga)',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `can_be_refined` tinyint(1) DEFAULT '0' COMMENT 'Se o item pode ser refinado de +0 a +12',
  `tradeable` tinyint(1) DEFAULT '1' COMMENT 'Se o item pode ser negociado (loja/leilão/trade)',
  `item_category` enum('equipment','consumable','material','upgrade','quest','misc') COLLATE utf8mb4_unicode_ci DEFAULT 'misc' COMMENT 'Categoria do item',
  PRIMARY KEY (`item_id`),
  KEY `idx_item_type` (`item_type`),
  KEY `idx_item_subtype` (`item_subtype`),
  KEY `idx_rarity` (`rarity`),
  KEY `idx_can_be_refined` (`can_be_refined`),
  KEY `idx_tradeable` (`tradeable`),
  KEY `idx_item_category` (`item_category`)
) ENGINE=InnoDB AUTO_INCREMENT=57 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `item_templates`
--

LOCK TABLES `item_templates` WRITE;
/*!40000 ALTER TABLE `item_templates` DISABLE KEYS */;
INSERT INTO `item_templates` VALUES (1,'Espada de Ferro','Uma espada comum feita de ferro.','weapon','sword','/Game/UI/Icons/Items/ICO_IronSword',1,'main_hand',1,'{\"attack\": 100, \"defense\": 0, \"accuracy\": 12, \"critical\": 5, \"strength\": 5, \"dexterity\": 5, \"health_bonus\": 50, \"intelligence\": 0, \"magic_attack\": 25, \"magic_defense\": 0, \"double_attack_rate\": 5}',5000,'uncommon',5000,0,'2025-11-14 17:47:15','2026-04-29 02:38:33',1,1,'equipment'),(2,'Espada Flamejante','Uma espada lendária envolta em chamas eternas.','weapon','sword','/Game/UI/Icons/Items/ICO_FlameSword',1,'main_hand',20,'{\"damage\": 50, \"fire_damage\": 25, \"attack_speed\": 1.5}',5000,'legendary',5000,0,'2025-11-14 17:47:15','2026-04-27 21:24:10',1,1,'equipment'),(3,'Arco Longo','Um arco de madeira resistente.','weapon','bow','/Game/UI/Icons/Items/ICO_LongBow',1,'main_hand',5,'{\"range\": 30, \"damage\": 20, \"attack_speed\": 0.8}',5000,'uncommon',120,0,'2025-11-14 17:47:15','2026-04-27 21:24:10',1,1,'equipment'),(4,'Capacete de Couro','Um capacete simples de couro.','armor','helmet','/Game/UI/Icons/Items/ICO_LeatherHelmet',1,'head',1,'{\"luck\": 0, \"dodge\": 0, \"attack\": 0, \"defense\": 20, \"accuracy\": 0, \"critical\": 0, \"movement\": 25, \"strength\": 5, \"vitality\": 5, \"dexterity\": 0, \"mana_bonus\": 0, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 0, \"magic_attack\": 0, \"magic_defense\": 15, \"double_attack_rate\": 0}',5000,'common',30,0,'2025-11-14 17:47:15','2026-04-27 21:24:10',1,1,'equipment'),(5,'Peitoral de Aço','Uma armadura de aço que protege o torso.','armor','chestplate','/Game/UI/Icons/Items/ICO_SteelChestplate',1,'chest',1,'{\"health\": 50, \"defense\": 25}',5000,'rare',500,0,'2025-11-14 17:47:15','2026-04-27 21:24:10',1,1,'equipment'),(6,'Botas Élficas','Botas leves que aumentam a velocidade.','weapon','boots','/Game/UI/Icons/Items/ICO_ElvenBoots',1,'feet',1,'{\"speed\": 15, \"defense\": 8}',5000,'epic',800,0,'2025-11-14 17:47:15','2026-04-27 21:24:10',1,1,'equipment'),(7,'Poção de Vida Menor','Restaura 150 pontos de vida.','consumable','health_potion','/Game/UI/Icons/Items/ICO_HealthPotionSmall',999,'none',1,'{\"health_restore\": 150}',5000,'common',10,0,'2025-11-14 17:47:15','2026-05-25 19:42:13',0,1,'consumable'),(8,'Poção de Vida Maior','Restaura 150 pontos de vida.','consumable','health_potion','/Game/UI/Icons/Items/ICO_HealthPotionLarge',99,'none',5,'{\"health_restore\": 150}',5000,'uncommon',50,0,'2025-11-14 17:47:15','2026-05-21 15:45:27',0,1,'consumable'),(9,'Poção de Mana','Restaura 100 pontos de mana.','consumable','mana_potion','/Game/UI/Icons/Items/ICO_ManaPotion',99,'none',1,'{\"mana_restore\": 100}',5000,'common',15,0,'2025-11-14 17:47:15','2026-05-21 15:45:40',0,1,'consumable'),(10,'Elixir de Força','Aumenta força em +20 por 5 minutos.','consumable','buff_potion','/Game/UI/Icons/Items/ICO_StrengthElixir',99,'none',1,'{\"duration\": 300, \"strength_buff\": 20}',1000,'uncommon',100,0,'2025-11-14 17:47:15','2026-05-21 15:46:21',0,1,'consumable'),(11,'Elixir de Destreza','Aumenta destreza em +20 por 5 minutos.','consumable','buff_potion','/Game/UI/Icons/Items/ICO_DexterityElixir',99,'none',1,'{\"duration\": 300, \"dexterity_buff\": 20}',1000,'uncommon',5,0,'2025-11-14 17:47:15','2026-05-21 15:36:24',0,1,'consumable'),(12,'Elixir de Inteligência','Aumenta inteligência em +20 por 5 minutos.','consumable','buff_potion','/Game/UI/Icons/Items/ICO_IntelligenceElixir',99,'none',1,'{\"duration\": 300, \"intelligence_buff\": 20}',1000,'uncommon',50,0,'2025-11-14 17:47:15','2026-05-21 15:48:56',0,1,'consumable'),(13,'Elixir de Vitalidade','Aumenta vitalidade em +20 por 5 minutos.','consumable','buff_potion','/Game/UI/Icons/Items/ICO_VitalityElixir',99,'none',1,'{\"duration\": 300, \"vitality_buff\": 20}',1000,'uncommon',8,0,'2025-11-14 17:47:15','2026-05-21 15:48:26',0,1,'consumable'),(14,'Elixir de Sorte','Aumenta sorte em +20 por 5 minutos.','consumable','buff_potion','/Game/UI/Icons/Items/ICO_LuckElixir',99,'none',1,'{\"duration\": 300, \"luck_buff\": 20}',1000,'uncommon',0,0,'2025-11-14 17:47:15','2026-05-21 15:47:42',0,1,'consumable'),(15,'Tônico do Guerreiro','Aumenta ataque físico em +30 por 3 minutos.','quest','buff_potion','/Game/UI/Icons/Items/ICO_WarriorTonic',99,'none',1,'{\"duration\": 180, \"attack_buff\": 30}',1000,'uncommon',0,0,'2025-11-14 17:47:15','2026-05-21 15:49:52',0,1,'quest'),(16,'Tônico da Defesa','Aumenta defesa física em +30 por 3 minutos.','misc','buff_potion','/Game/UI/Icons/Items/ICO_DefenseTonic',99,'none',1,'{\"duration\": 180, \"defense_buff\": 30}',1000,'common',1,0,'2025-11-14 17:47:15','2026-05-21 15:50:04',0,1,'misc'),(17,'Gema Brilhante','Uma gema valiosa. Pode ser vendida por um bom preço.','misc','gem','/Game/UI/Icons/Items/ICO_BrightGem',99,'none',1,'{}',5000,'rare',200,0,'2025-11-14 17:47:15','2025-11-14 17:47:15',0,1,'misc'),(35,'Anel do Mestre','Um anel único pertencente somente ao mestre do universo. Concede poderes imensuráveis.','armor','ring','/Game/UI/Icons/Items/ICO_MasterRing',1,'ring',1,'{\"luck\": 50, \"dodge\": 50, \"attack\": 100, \"defense\": 100, \"accuracy\": 50, \"critical\": 50, \"movement\": 25, \"strength\": 50, \"vitality\": 50, \"dexterity\": 50, \"mana_bonus\": 500, \"resistance\": 50, \"health_bonus\": 500, \"intelligence\": 50, \"magic_attack\": 100, \"magic_defense\": 100, \"double_attack_rate\": 50, \"double_attack_resistance\": 50}',5000,'legendary',999999,0,'2025-11-28 20:08:53','2026-04-27 21:24:10',1,1,'equipment'),(36,'Amuleto do Mestre','Um amuleto único pertencente somente ao mestre do universo. Concede poderes imensuráveis.','armor','amulet','/Game/UI/Icons/Items/ICO_MasterAmulet',1,'amulet',1,'{\"luck\": 50, \"dodge\": 50, \"attack\": 100, \"defense\": 100, \"accuracy\": 50, \"critical\": 50, \"movement\": 25, \"strength\": 50, \"vitality\": 50, \"dexterity\": 50, \"mana_bonus\": 500, \"resistance\": 50, \"health_bonus\": 500, \"intelligence\": 50, \"magic_attack\": 100, \"magic_defense\": 100, \"double_attack_rate\": 50, \"double_attack_resistance\": 50}',5000,'legendary',999999,0,'2025-11-28 20:08:53','2026-04-27 21:24:10',1,1,'equipment'),(37,'Colar do Mestre','Um colar único pertencente somente ao mestre do universo. Concede poderes imensuráveis.','armor','necklace','/Game/UI/Icons/Items/ICO_MasterNecklace',1,'necklace',1,'{\"luck\": 50, \"dodge\": 50, \"attack\": 100, \"defense\": 100, \"accuracy\": 50, \"critical\": 50, \"movement\": 25, \"strength\": 50, \"vitality\": 50, \"dexterity\": 50, \"mana_bonus\": 500, \"resistance\": 50, \"health_bonus\": 500, \"intelligence\": 50, \"magic_attack\": 100, \"magic_defense\": 100, \"double_attack_rate\": 50, \"double_attack_resistance\": 50}',5000,'epic',9999999,0,'2025-11-28 20:08:53','2026-04-27 21:24:10',1,1,'equipment'),(38,'Brincos do Mestre','Brincos únicos pertencentes somente ao mestre do universo. Concedem poderes imensuráveis.','armor','earring','/Game/UI/Icons/Items/ICO_MasterEarring',1,'earring',1,'{\"luck\": 50, \"dodge\": 50, \"attack\": 100, \"defense\": 100, \"accuracy\": 50, \"critical\": 50, \"movement\": 25, \"strength\": 50, \"vitality\": 50, \"dexterity\": 50, \"mana_bonus\": 500, \"resistance\": 50, \"health_bonus\": 500, \"intelligence\": 50, \"magic_attack\": 100, \"magic_defense\": 100, \"double_attack_rate\": 50, \"double_attack_resistance\": 50}',5000,'legendary',999999,0,'2025-11-28 20:08:53','2026-04-27 21:24:10',1,1,'equipment'),(39,'Luvas de Couro','Luvas simples de couro.','armor','hands','/Game/UI/Icons/Items/ICO_Leather_Gloves',1,'hands',1,'{\"defense\": 14, \"strength\": 2, \"vitality\": 2, \"resistance\": 1, \"health_bonus\": 20, \"magic_defense\": 8}',5000,'common',20,0,'2025-11-30 13:10:35','2026-04-27 21:24:10',1,1,'equipment'),(40,'Anel de Energia Umbral','Anel com energia umbral condensada. Amplia os poderes do usuário.','armor','Ring','/Game/UI/Icons/Items/ICO_Umbral_Energy_Ring',1,'ring',1,'{\"luck\": 5, \"dodge\": 5, \"attack\": 20, \"defense\": 20, \"accuracy\": 5, \"critical\": 5, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 50, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 5, \"magic_attack\": 20, \"magic_defense\": 20, \"double_attack_rate\": 5}',5000,'uncommon',5,0,'2025-12-01 05:06:27','2026-04-27 21:24:10',1,1,'equipment'),(41,'Botas de Metal Mágico Umbral','Botas pesadas feitas de metal mágico e compostas com energia umbral, são robustas e protegem o usuário.','armor','feet','/Game/UI/Icons/Items/ICO_Draconic_Boots',1,'feet',1,'{\"luck\": 5, \"dodge\": 5, \"attack\": 10, \"defense\": 100, \"accuracy\": 5, \"critical\": 5, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 50, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 5, \"magic_attack\": 10, \"magic_defense\": 100, \"double_attack_rate\": 5}',5000,'epic',999999,0,'2025-12-01 05:16:18','2026-07-01 04:27:54',1,1,'equipment'),(42,'Capacete de Metal Mágico Umbral','Capacete feito de metal mágico e composto com energia umbral, é robusto e protege o usuário.','weapon','head','/Game/UI/Icons/Items/ICO_Umbral_Magic_Metal_Helmet',1,'head',1,'{\"luck\": 5, \"dodge\": 5, \"attack\": 5, \"defense\": 100, \"accuracy\": 5, \"critical\": 5, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 50, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 5, \"magic_attack\": 5, \"magic_defense\": 100, \"double_attack_rate\": 5}',5000,'epic',999999,0,'2025-12-01 05:24:53','2026-04-27 21:24:10',1,1,'equipment'),(44,'Peitoral de Metal Mágico Umbral','Peitoral feito de metal mágico e composto com energia umbral, é robusto e protege o usuário.','armor','chestplate','/Game/UI/Icons/Items/ICO_Magic_Metal_Umbral_Chestplate',1,'chest',1,'{\"luck\": 10, \"dodge\": 10, \"attack\": 1, \"defense\": 125, \"accuracy\": 1, \"critical\": 1, \"movement\": 1, \"strength\": 10, \"vitality\": 10, \"dexterity\": 10, \"mana_bonus\": 200, \"resistance\": 1, \"health_bonus\": 200, \"intelligence\": 10, \"magic_attack\": 1, \"magic_defense\": 98, \"double_attack_rate\": 1}',5000,'epic',1,0,'2025-12-01 17:36:39','2026-04-27 21:24:10',1,1,'equipment'),(45,'Luvas de Metal Mágico Umbral','Luvas pesadas feitas de metal mágico e composto com energia umbral, são robustas e protegem o usuário.','armor','gloves','/Game/UI/Icons/Items/Icon_Umbral_Magic_Metal_Gloves',1,'hands',1,'{\"luck\": 5, \"dodge\": 5, \"attack\": 5, \"defense\": 100, \"accuracy\": 5, \"critical\": 5, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 50, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 5, \"magic_attack\": 5, \"magic_defense\": 100, \"double_attack_rate\": 5}',5000,'legendary',5,0,'2025-12-01 17:46:07','2026-04-27 21:24:10',1,1,'equipment'),(46,'Espada de Aço Reforçado','Espada criado por um ótimo artesão, é reforçada, e apenas aqueles com contribuições importantes são capazes de obtê-la.','weapon','sword','/Game/UI/Icons/Items/Icon_Reiforced_Steel_Sword',1,'main_hand',1,'{\"attack\": 175, \"accuracy\": 22, \"critical\": 12, \"strength\": 10, \"dexterity\": 5, \"health_bonus\": 75, \"magic_attack\": 48, \"double_attack_rate\": 6}',5000,'rare',75,0,'2025-12-01 17:57:11','2026-04-27 21:24:10',1,1,'equipment'),(47,'Amuleto Dracônico','Amuleto místico antigo, dizem conter partes do poder selado de um dragão a muito tempo esquecido','armor','amulet','/Game/UI/Icons/Items/Icon_Draconic_Amulet',1,'amulet',1,'{\"dodge\": 8, \"attack\": 75, \"accuracy\": 12, \"critical\": 8, \"strength\": 10, \"vitality\": 10, \"mana_bonus\": 75, \"health_bonus\": 75, \"intelligence\": 10, \"magic_attack\": 75, \"double_attack_rate\": 4}',5000,'epic',75,0,'2025-12-01 23:23:37','2026-04-27 21:24:10',1,1,'equipment'),(48,'Colar de Poder Mágico Dracônico','Colar criado por um feiticeiro que se alimentava da Energia Umbral. Aumenta os poderes mágicos do usuário.','armor','necklace','/Game/UI/Icons/Items/Icon_Magic_Draconic_Necklace',1,'necklace',1,'{\"luck\": 5, \"dodge\": 5, \"attack\": 5, \"defense\": 10, \"accuracy\": 12, \"critical\": 4, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 500, \"resistance\": 5, \"health_bonus\": 50, \"intelligence\": 25, \"magic_attack\": 125, \"magic_defense\": 100, \"double_attack_rate\": 0}',5000,'epic',50000,0,'2025-12-02 00:29:24','2026-04-27 21:24:10',1,1,'equipment'),(49,'Brincos de Magia Dracônica','Brincos mágicos infundidos com poder de almas dracônicas. Fortalece o poder mágico do usuário.','armor','earrings','/Game/UI/Icons/Items/Icon_Draconic_Magic_Earrings',1,'earring',1,'{\"luck\": 10, \"dodge\": 12, \"attack\": 5, \"defense\": 10, \"accuracy\": 12, \"critical\": 8, \"movement\": 5, \"strength\": 5, \"vitality\": 5, \"dexterity\": 5, \"mana_bonus\": 500, \"resistance\": 10, \"health_bonus\": 50, \"intelligence\": 50, \"magic_attack\": 100, \"magic_defense\": 100, \"double_attack_rate\": 10}',5000,'epic',50000,0,'2025-12-02 02:15:39','2026-04-27 21:24:10',1,1,'equipment'),(50,'Anel de Magia Dracônico','Um anel forjado por magos que buscavam obter o poder o almas dracônicas. Fortalece a magia do usuário.','armor','ring','/Game/UI/Icons/Items/Icon_Magic_Draconic_Ring',1,'ring',1,'{\"accuracy\": 12, \"critical\": 8, \"vitality\": 10, \"mana_bonus\": 250, \"intelligence\": 25, \"magic_attack\": 125, \"magic_defense\": 75, \"double_attack_rate\": 4}',5000,'epic',250,0,'2025-12-02 12:03:58','2026-04-27 21:24:10',1,1,'equipment'),(51,'Manoplas do Conselho de Grimholt','Manoplas feitas para os membros do conselho de Grimholt','armor','gloves','/Game/UI/Icons/Items/Icon_Grimholt_Counsil_Gloves',1,'hands',1,'{\"defense\": 78, \"strength\": 4, \"vitality\": 4, \"dexterity\": 8, \"mana_bonus\": 75, \"resistance\": 4, \"health_bonus\": 75, \"magic_defense\": 94}',5000,'uncommon',75,0,'2025-12-02 16:55:30','2026-04-27 21:24:10',1,1,'equipment'),(52,'Fragmento de Energia Umbral','Um fragmento cristalizado de energia sombria. Usado para refinar equipamentos de +0 a +6. A energia umbral flui através dele, fortalecendo armas e armaduras.','material','upgrade_material','/Game/UI/Icons/Items/ICO_UmbralFragment',999,'none',1,NULL,5000,'uncommon',100,0.1,'2026-04-27 21:24:10','2026-04-27 21:24:10',0,1,'upgrade'),(53,'Pedra de Energia Umbral','Uma pedra de energia concentrada, pulsando com poder sombrio. Necessária para refinamentos avançados de +7 a +12. Extremamente rara e valiosa.','material','upgrade_material','/Game/UI/Icons/Items/ICO_UmbralStone',999,'none',1,NULL,5000,'rare',500,0.2,'2026-04-27 21:24:10','2026-04-27 21:24:10',0,1,'upgrade'),(55,'Poção de força teste','Uma poção para teste','consumable','buff_potion','/Game/UI/Icons/Itens/ICO_pot',999,'none',1,NULL,5000,'rare',10,0,'2026-05-31 00:11:35','2026-05-31 00:11:35',0,1,'consumable'),(56,'Botina de Ferro','Uma botina de ferro','armor','boots','/Game/UI....',1,'feet',10,NULL,5000,'legendary',55555,0,'2026-06-21 16:16:01','2026-06-21 16:16:01',1,1,'equipment');
/*!40000 ALTER TABLE `item_templates` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_instances`
--

DROP TABLE IF EXISTS `npc_instances`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `npc_instances` (
  `npc_instance_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `npc_template_id` bigint unsigned NOT NULL,
  `zone_id` int unsigned NOT NULL DEFAULT '1',
  `pos_x` float NOT NULL DEFAULT '0',
  `pos_y` float NOT NULL DEFAULT '0',
  `pos_z` float NOT NULL DEFAULT '200',
  `yaw` float NOT NULL DEFAULT '0',
  `current_health` int NOT NULL DEFAULT '100',
  `current_mana` int NOT NULL DEFAULT '50',
  `is_dead` tinyint(1) NOT NULL DEFAULT '0',
  `respawn_at` timestamp NULL DEFAULT NULL,
  `last_combat_at` timestamp NULL DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`npc_instance_id`),
  KEY `idx_npc_instances_zone` (`zone_id`),
  KEY `idx_npc_instances_template` (`npc_template_id`),
  CONSTRAINT `fk_npc_instances_template` FOREIGN KEY (`npc_template_id`) REFERENCES `npc_templates` (`npc_template_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `npc_instances`
--

LOCK TABLES `npc_instances` WRITE;
/*!40000 ALTER TABLE `npc_instances` DISABLE KEYS */;
INSERT INTO `npc_instances` VALUES (1,1,0,-1347,-1310,67,0,10334559,1000,0,NULL,'2026-07-05 22:30:41','2026-06-20 02:53:34','2026-07-05 22:30:41'),(2,2,0,-1118,-1714,62,0,49182544,25000000,0,NULL,'2026-07-04 04:58:12','2026-06-20 02:54:19','2026-07-04 04:58:12'),(3,5,0,965,-963,78,180,1000,100,0,NULL,NULL,'2026-07-02 18:57:30','2026-07-02 19:44:48'),(4,6,0,1488,-3453,60,0,1000,100,0,NULL,'2026-07-06 18:08:14','2026-07-03 21:10:35','2026-07-06 18:08:29'),(5,7,0,1537,-4044,101,90,1200,200,0,NULL,'2026-07-05 18:38:08','2026-07-03 23:08:34','2026-07-05 18:38:38'),(6,9,0,-3522,2121,485,90,1000,100,0,NULL,NULL,'2026-07-04 03:49:11','2026-07-04 03:49:11'),(10,8,0,1012,807,69,90,1000,100,0,NULL,NULL,'2026-07-03 23:28:58','2026-07-03 23:28:58'),(11,10,0,-13834,2790,867,180,2000,350,0,NULL,'2026-07-05 02:52:21','2026-07-04 04:11:41','2026-07-05 02:52:51'),(12,10,0,-15621,3853,919,90,2000,350,0,NULL,'2026-07-05 02:53:02','2026-07-04 04:29:04','2026-07-05 02:53:32'),(13,10,0,-17704,329,751,0,2000,350,0,NULL,'2026-07-05 01:18:40','2026-07-04 04:31:24','2026-07-05 01:19:10'),(14,10,0,-18049,2605,675,180,2000,350,0,NULL,'2026-07-05 01:18:29','2026-07-04 04:39:07','2026-07-05 01:18:59'),(15,10,0,-17599,7695,486,120,2000,350,0,NULL,'2026-07-05 01:18:13','2026-07-04 04:40:38','2026-07-05 01:18:44');
/*!40000 ALTER TABLE `npc_instances` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_quest_offers`
--

DROP TABLE IF EXISTS `npc_quest_offers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `npc_quest_offers` (
  `offer_id` int unsigned NOT NULL AUTO_INCREMENT,
  `npc_template_id` bigint unsigned NOT NULL,
  `quest_id` int unsigned NOT NULL,
  `sort_order` int NOT NULL DEFAULT '0',
  `is_quest_giver` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`offer_id`),
  UNIQUE KEY `uk_npc_quest_offer` (`npc_template_id`,`quest_id`),
  KEY `idx_npc_quest_offers_quest` (`quest_id`),
  CONSTRAINT `fk_npc_quest_offers_quest` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `npc_quest_offers`
--

LOCK TABLES `npc_quest_offers` WRITE;
/*!40000 ALTER TABLE `npc_quest_offers` DISABLE KEYS */;
INSERT INTO `npc_quest_offers` VALUES (1,5,1,0,1),(2,5,2,1,1),(3,5,3,2,1),(4,9,4,0,1);
/*!40000 ALTER TABLE `npc_quest_offers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_templates`
--

DROP TABLE IF EXISTS `npc_templates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `npc_templates` (
  `npc_template_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `npc_name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `level` int unsigned NOT NULL DEFAULT '1',
  `max_health` int unsigned NOT NULL DEFAULT '100',
  `max_mana` int unsigned NOT NULL DEFAULT '50',
  `strength` int unsigned NOT NULL DEFAULT '10',
  `dexterity` int unsigned NOT NULL DEFAULT '10',
  `vitality` int unsigned NOT NULL DEFAULT '10',
  `intelligence` int unsigned NOT NULL DEFAULT '10',
  `luck` int unsigned NOT NULL DEFAULT '10',
  `physical_attack` int NOT NULL DEFAULT '0',
  `magic_attack` int NOT NULL DEFAULT '0',
  `physical_defense` int NOT NULL DEFAULT '0',
  `magic_defense` int NOT NULL DEFAULT '0',
  `accuracy` int NOT NULL DEFAULT '0',
  `dodge` int NOT NULL DEFAULT '0',
  `critical` int NOT NULL DEFAULT '0',
  `critical_resistance` int NOT NULL DEFAULT '0',
  `double_attack_rate` int NOT NULL DEFAULT '0',
  `double_attack_resistance` int NOT NULL DEFAULT '0',
  `skeletal_mesh_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `anim_blueprint_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `is_editable` tinyint(1) NOT NULL DEFAULT '1',
  `is_attackable` tinyint(1) NOT NULL DEFAULT '1',
  `interaction_radius` float NOT NULL DEFAULT '300',
  `has_vendor` tinyint(1) NOT NULL DEFAULT '0',
  `has_quest_dialog` tinyint(1) NOT NULL DEFAULT '0',
  `dialog_title` varchar(120) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `dialog_text` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `respawn_seconds` int unsigned NOT NULL DEFAULT '30' COMMENT 'Segundos até respawn após morte',
  PRIMARY KEY (`npc_template_id`),
  KEY `idx_npc_templates_name` (`npc_name`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `npc_templates`
--

LOCK TABLES `npc_templates` WRITE;
/*!40000 ALTER TABLE `npc_templates` DISABLE KEYS */;
INSERT INTO `npc_templates` VALUES (1,'dummy_treino',1,15000000,1000,20,20,20,20,20,120,120,80,80,100,20,5,5,5,5,NULL,NULL,1,1,300,0,0,NULL,NULL,'2026-06-01 04:01:46','2026-07-03 21:19:59',15),(2,'dummy_upado',50,50000000,25000000,200,200,200,200,200,1200,1200,1200,1200,100,200,100,200,100,200,'','',1,1,300,0,0,NULL,NULL,'2026-06-20 00:31:42','2026-06-22 03:29:35',30),(3,'dummy_treino',1,5000,1000,20,20,20,20,20,120,120,80,80,100,20,5,5,5,5,NULL,NULL,1,1,300,0,0,NULL,NULL,'2026-06-20 01:38:34','2026-07-03 21:19:59',15),(4,'Dummy',1,5000,1000,20,20,20,20,20,120,120,80,80,100,20,5,5,5,5,'/Game/Characters/Mannequins/Meshes/SKM_Manny','/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed',1,1,300,0,0,NULL,NULL,'2026-06-20 02:07:12','2026-06-20 02:07:12',30),(5,'npc_merchant_01',1,1000,100,5,5,5,5,5,0,0,10,10,0,0,0,0,0,0,'/Game/Characters/Mannequins/Meshes/SKM_Manny','/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed',1,0,300,1,1,'Mercador do Vilarejo','Bem-vindo! Posso oferecer missões, comércio ou apenas conversar.','2026-07-02 18:57:30','2026-07-03 15:56:18',30),(6,'mob_quest_01',1,1000,100,5,5,5,5,5,0,0,10,10,0,0,0,0,0,0,NULL,NULL,1,1,300,0,0,'','','2026-07-03 20:58:40','2026-07-03 21:19:59',15),(7,'mob_quest_02',1,1200,200,20,20,20,20,20,150,150,100,100,50,30,30,30,30,30,NULL,NULL,1,1,300,0,0,NULL,NULL,'2026-07-03 23:04:55','2026-07-03 23:05:20',30),(8,'Npc_Potion_Merchant',1,1000,100,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,NULL,NULL,1,0,300,1,1,'Mercador de Poções','Bem-vindo, aventureiro! Estou com a seleção dos melhores elixires!','2026-07-03 23:21:50','2026-07-03 23:21:50',30),(9,'Cidadão da Vila',1,1000,100,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,NULL,NULL,1,0,300,0,1,'Cidadão em Apuros','Ei, você! Você parece ser um aventureiro. Minha amada foi atacada pelos malditos goblins, você pode vingá-la por mim?','2026-07-04 03:47:35','2026-07-04 03:47:35',30),(10,'Goblin',2,2000,350,25,25,25,25,25,125,125,175,145,35,35,35,35,35,35,NULL,NULL,1,1,300,0,0,NULL,NULL,'2026-07-04 03:58:53','2026-07-04 03:58:53',30);
/*!40000 ALTER TABLE `npc_templates` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_vendor_stock`
--

DROP TABLE IF EXISTS `npc_vendor_stock`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `npc_vendor_stock` (
  `stock_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `vendor_id` bigint unsigned NOT NULL,
  `item_template_id` int NOT NULL,
  `buy_price_gold` int unsigned NOT NULL,
  `stock_qty` int NOT NULL DEFAULT '-1',
  `max_buy_per_tx` int unsigned NOT NULL DEFAULT '99',
  `sort_order` int NOT NULL DEFAULT '0',
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`stock_id`),
  UNIQUE KEY `uk_vendor_item` (`vendor_id`,`item_template_id`),
  KEY `idx_vendor_stock_vendor` (`vendor_id`),
  KEY `fk_vendor_stock_item` (`item_template_id`),
  CONSTRAINT `fk_vendor_stock_item` FOREIGN KEY (`item_template_id`) REFERENCES `item_templates` (`item_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_vendor_stock_vendor` FOREIGN KEY (`vendor_id`) REFERENCES `npc_vendors` (`vendor_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `npc_vendor_stock`
--

LOCK TABLES `npc_vendor_stock` WRITE;
/*!40000 ALTER TABLE `npc_vendor_stock` DISABLE KEYS */;
INSERT INTO `npc_vendor_stock` VALUES (1,1,1,100,9999998,99,0,1,'2026-07-02 18:57:30','2026-07-03 16:50:48'),(2,1,9,100,999999755,99,1,1,'2026-07-02 18:57:30','2026-07-04 01:05:06'),(3,1,10,120,99999751,99,2,1,'2026-07-02 18:57:30','2026-07-03 21:32:24'),(4,1,7,30,999849,99,3,1,'2026-07-02 18:57:30','2026-07-05 02:53:45'),(5,1,5,500,999999998,99,4,1,'2026-07-02 18:57:30','2026-07-03 16:50:52'),(6,2,7,50,99999960,99,0,1,'2026-07-03 23:25:50','2026-07-04 03:31:45'),(7,2,9,50,9999999,99,1,1,'2026-07-03 23:36:01','2026-07-03 23:36:01'),(8,2,10,250,999999999,99,2,1,'2026-07-03 23:37:48','2026-07-03 23:37:48'),(9,2,11,250,999999999,99,3,1,'2026-07-03 23:38:18','2026-07-03 23:38:18'),(10,2,12,250,99999999,99,4,1,'2026-07-03 23:39:21','2026-07-03 23:39:21'),(11,2,13,250,999999983,99,5,1,'2026-07-03 23:40:22','2026-07-03 23:41:47'),(12,2,14,250,99999968,99,6,1,'2026-07-03 23:40:52','2026-07-04 15:04:45');
/*!40000 ALTER TABLE `npc_vendor_stock` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `npc_vendors`
--

DROP TABLE IF EXISTS `npc_vendors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `npc_vendors` (
  `vendor_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `npc_template_id` bigint unsigned NOT NULL,
  `vendor_display_name` varchar(100) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `sell_rate_percent` tinyint unsigned NOT NULL DEFAULT '50',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`vendor_id`),
  UNIQUE KEY `uk_npc_vendor_template` (`npc_template_id`),
  CONSTRAINT `fk_npc_vendors_template` FOREIGN KEY (`npc_template_id`) REFERENCES `npc_templates` (`npc_template_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `npc_vendors`
--

LOCK TABLES `npc_vendors` WRITE;
/*!40000 ALTER TABLE `npc_vendors` DISABLE KEYS */;
INSERT INTO `npc_vendors` VALUES (1,5,'Mercador do Vilarejo',50,'2026-07-02 18:57:30'),(2,8,'Vendedor de Poções',50,'2026-07-03 23:24:13');
/*!40000 ALTER TABLE `npc_vendors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `parties`
--

DROP TABLE IF EXISTS `parties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `parties` (
  `party_id` int unsigned NOT NULL AUTO_INCREMENT,
  `leader_id` bigint unsigned NOT NULL COMMENT 'Líder do grupo',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`party_id`),
  KEY `idx_leader` (`leader_id`),
  CONSTRAINT `parties_ibfk_1` FOREIGN KEY (`leader_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=163 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `parties`
--

LOCK TABLES `parties` WRITE;
/*!40000 ALTER TABLE `parties` DISABLE KEYS */;
INSERT INTO `parties` VALUES (4,30,'2026-02-09 01:15:02','2026-02-09 03:37:33'),(84,23,'2026-02-26 15:10:27','2026-02-26 15:10:27');
/*!40000 ALTER TABLE `parties` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `party_invites`
--

DROP TABLE IF EXISTS `party_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `party_invites` (
  `invite_id` int unsigned NOT NULL AUTO_INCREMENT,
  `party_id` int unsigned DEFAULT NULL COMMENT 'ID do grupo (NULL = convite pendente)',
  `from_player_id` bigint unsigned NOT NULL COMMENT 'Jogador que enviou o convite',
  `to_player_id` bigint unsigned NOT NULL COMMENT 'Jogador que recebeu o convite',
  `status` enum('pending','accepted','declined','expired') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `responded_at` timestamp NULL DEFAULT NULL,
  `expires_at` timestamp NULL DEFAULT NULL COMMENT 'Convite expira após 5 minutos',
  PRIMARY KEY (`invite_id`),
  KEY `idx_from_player` (`from_player_id`),
  KEY `idx_to_player` (`to_player_id`),
  KEY `idx_party_id` (`party_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `party_invites_ibfk_1` FOREIGN KEY (`from_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `party_invites_ibfk_2` FOREIGN KEY (`to_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=267 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `party_invites`
--

LOCK TABLES `party_invites` WRITE;
/*!40000 ALTER TABLE `party_invites` DISABLE KEYS */;
INSERT INTO `party_invites` VALUES (1,NULL,1,23,'expired','2026-02-08 22:21:09',NULL,'2026-02-09 01:26:09'),(2,NULL,23,1,'expired','2026-02-08 22:21:21',NULL,'2026-02-09 01:26:21'),(10,NULL,23,1,'expired','2026-02-09 00:43:18',NULL,'2026-02-09 03:44:18'),(11,1,1,23,'accepted','2026-02-09 00:43:28','2026-02-09 00:43:31','2026-02-09 03:44:28'),(12,1,1,23,'expired','2026-02-09 00:46:56',NULL,'2026-02-09 03:47:56'),(13,1,23,1,'expired','2026-02-09 00:59:51',NULL,'2026-02-09 04:00:51'),(14,1,1,23,'expired','2026-02-09 01:12:22',NULL,'2026-02-09 04:13:22'),(15,1,23,1,'expired','2026-02-09 01:12:34',NULL,'2026-02-09 04:13:34'),(16,4,25,30,'accepted','2026-02-09 01:15:00','2026-02-09 01:15:02','2026-02-09 04:16:00'),(17,1,23,1,'expired','2026-02-09 01:38:30',NULL,'2026-02-09 04:39:30'),(18,1,23,1,'expired','2026-02-09 01:41:23',NULL,'2026-02-09 04:42:23'),(19,1,1,23,'expired','2026-02-09 02:06:18',NULL,'2026-02-09 05:07:18'),(20,5,1,23,'accepted','2026-02-09 02:07:02','2026-02-09 02:07:05','2026-02-09 05:08:02'),(21,6,1,23,'accepted','2026-02-09 03:06:00','2026-02-09 03:06:02','2026-02-09 06:07:00'),(22,7,1,23,'accepted','2026-02-09 03:19:43','2026-02-09 03:19:46','2026-02-09 06:20:43'),(23,8,23,1,'accepted','2026-02-09 03:37:15','2026-02-09 03:37:18','2026-02-09 06:38:15'),(24,9,1,23,'accepted','2026-02-09 03:41:30','2026-02-09 03:41:33','2026-02-09 06:42:30'),(25,9,1,14,'accepted','2026-02-09 03:41:38','2026-02-09 03:41:41','2026-02-09 06:42:38'),(26,9,1,14,'expired','2026-02-09 03:41:56',NULL,'2026-02-09 06:42:56'),(27,9,1,14,'expired','2026-02-09 03:42:21',NULL,'2026-02-09 06:43:21'),(28,10,1,4,'accepted','2026-02-09 12:59:08','2026-02-09 12:59:11','2026-02-09 16:00:08'),(29,10,1,23,'accepted','2026-02-09 12:59:20','2026-02-09 12:59:23','2026-02-09 16:00:20'),(30,10,1,23,'accepted','2026-02-09 13:47:36','2026-02-09 13:47:39','2026-02-09 16:48:36'),(31,11,4,1,'accepted','2026-02-09 15:28:19','2026-02-09 15:28:22','2026-02-09 18:29:19'),(32,11,4,23,'accepted','2026-02-09 15:28:30','2026-02-09 15:28:35','2026-02-09 18:29:30'),(33,12,1,23,'accepted','2026-02-09 15:54:11','2026-02-09 15:54:15','2026-02-09 18:55:11'),(34,12,1,4,'accepted','2026-02-09 15:54:31','2026-02-09 15:54:36','2026-02-09 18:55:31'),(35,13,4,23,'accepted','2026-02-09 16:28:53','2026-02-09 16:28:58','2026-02-09 19:29:53'),(36,13,4,1,'accepted','2026-02-09 16:29:04','2026-02-09 16:29:07','2026-02-09 19:30:04'),(37,14,1,23,'accepted','2026-02-09 16:43:12','2026-02-09 16:43:15','2026-02-09 19:44:12'),(38,14,1,4,'accepted','2026-02-09 16:43:25','2026-02-09 16:43:28','2026-02-09 19:44:25'),(39,15,1,4,'accepted','2026-02-09 16:59:11','2026-02-09 16:59:14','2026-02-09 20:00:11'),(40,15,1,23,'accepted','2026-02-09 16:59:23','2026-02-09 16:59:26','2026-02-09 20:00:23'),(41,16,1,23,'accepted','2026-02-09 17:28:14','2026-02-09 17:28:17','2026-02-09 20:29:14'),(42,16,1,4,'accepted','2026-02-09 17:28:23','2026-02-09 17:28:25','2026-02-09 20:29:23'),(43,16,1,23,'accepted','2026-02-09 17:44:25','2026-02-09 17:44:28','2026-02-09 20:45:25'),(44,17,1,23,'accepted','2026-02-09 18:16:55','2026-02-09 18:16:58','2026-02-09 21:17:55'),(45,17,1,4,'accepted','2026-02-09 18:17:18','2026-02-09 18:17:20','2026-02-09 21:18:18'),(46,18,1,4,'accepted','2026-02-09 18:31:56','2026-02-09 18:31:58','2026-02-09 21:32:56'),(47,18,1,23,'accepted','2026-02-09 18:32:08','2026-02-09 18:32:10','2026-02-09 21:33:08'),(48,18,23,1,'accepted','2026-02-09 18:32:30','2026-02-09 18:32:33','2026-02-09 21:33:30'),(49,18,1,23,'accepted','2026-02-09 18:32:54','2026-02-09 18:32:57','2026-02-09 21:33:54'),(50,19,1,4,'accepted','2026-02-09 18:46:27','2026-02-09 18:46:29','2026-02-09 21:47:27'),(51,19,1,23,'accepted','2026-02-09 18:46:34','2026-02-09 18:46:36','2026-02-09 21:47:34'),(52,19,1,4,'expired','2026-02-09 18:46:54',NULL,'2026-02-09 21:47:54'),(53,19,1,23,'accepted','2026-02-09 18:47:03','2026-02-09 18:47:06','2026-02-09 21:48:03'),(54,19,4,1,'accepted','2026-02-09 18:47:16','2026-02-09 18:47:19','2026-02-09 21:48:16'),(55,19,4,23,'expired','2026-02-09 18:47:30',NULL,'2026-02-09 21:48:30'),(56,19,4,1,'accepted','2026-02-09 18:47:44','2026-02-09 18:47:46','2026-02-09 21:48:44'),(57,20,1,23,'accepted','2026-02-09 18:51:06','2026-02-09 18:51:10','2026-02-09 21:52:06'),(58,20,1,14,'accepted','2026-02-09 18:51:19','2026-02-09 18:51:23','2026-02-09 21:52:19'),(59,20,1,4,'accepted','2026-02-09 18:51:34','2026-02-09 18:51:37','2026-02-09 21:52:34'),(60,20,23,1,'accepted','2026-02-09 18:55:44','2026-02-09 18:55:47','2026-02-09 21:56:44'),(61,21,1,23,'accepted','2026-02-09 19:46:07','2026-02-09 19:46:09','2026-02-09 22:47:07'),(62,21,1,4,'accepted','2026-02-09 19:46:20','2026-02-09 19:46:23','2026-02-09 22:47:20'),(63,22,1,23,'accepted','2026-02-09 20:05:47','2026-02-09 20:05:53','2026-02-09 23:06:47'),(64,22,1,4,'accepted','2026-02-09 20:06:02','2026-02-09 20:06:05','2026-02-09 23:07:02'),(65,23,14,23,'accepted','2026-02-09 20:21:18','2026-02-09 20:21:21','2026-02-09 23:22:18'),(66,23,14,4,'accepted','2026-02-09 20:21:34','2026-02-09 20:21:37','2026-02-09 23:22:34'),(67,23,14,1,'accepted','2026-02-09 20:21:50','2026-02-09 20:21:53','2026-02-09 23:22:50'),(68,24,1,23,'accepted','2026-02-09 22:31:42','2026-02-09 22:31:44','2026-02-10 01:32:42'),(69,24,23,1,'expired','2026-02-09 22:32:04',NULL,'2026-02-10 01:33:04'),(70,24,1,23,'expired','2026-02-09 22:32:10',NULL,'2026-02-10 01:33:10'),(71,25,1,23,'accepted','2026-02-09 22:32:41','2026-02-09 22:32:42','2026-02-10 01:33:41'),(72,26,1,23,'accepted','2026-02-09 22:41:27','2026-02-09 22:41:29','2026-02-10 01:42:27'),(73,27,1,23,'accepted','2026-02-09 22:45:08','2026-02-09 22:45:10','2026-02-10 01:46:08'),(74,28,1,23,'accepted','2026-02-09 22:48:27','2026-02-09 22:48:28','2026-02-10 01:49:27'),(75,29,1,23,'accepted','2026-02-09 23:12:43','2026-02-09 23:12:44','2026-02-10 02:13:43'),(76,30,1,23,'accepted','2026-02-09 23:13:40','2026-02-09 23:13:41','2026-02-10 02:14:40'),(77,31,1,23,'accepted','2026-02-09 23:16:31','2026-02-09 23:16:33','2026-02-10 02:17:31'),(78,32,1,23,'accepted','2026-02-09 23:17:21','2026-02-09 23:17:22','2026-02-10 02:18:21'),(79,33,1,23,'accepted','2026-02-09 23:28:03','2026-02-09 23:28:06','2026-02-10 02:29:03'),(80,34,1,23,'accepted','2026-02-09 23:33:02','2026-02-09 23:33:04','2026-02-10 02:34:02'),(81,35,1,23,'accepted','2026-02-09 23:41:11','2026-02-09 23:41:12','2026-02-10 02:42:11'),(82,36,23,1,'accepted','2026-02-10 02:05:20','2026-02-10 02:05:21','2026-02-10 05:06:20'),(83,37,1,23,'accepted','2026-02-10 02:41:48','2026-02-10 02:41:50','2026-02-10 05:42:48'),(84,38,23,1,'accepted','2026-02-10 02:50:19','2026-02-10 02:50:21','2026-02-10 05:51:19'),(85,39,23,1,'accepted','2026-02-10 12:03:04','2026-02-10 12:03:06','2026-02-10 15:04:04'),(86,40,23,1,'accepted','2026-02-10 13:19:50','2026-02-10 13:19:51','2026-02-10 16:20:50'),(87,41,1,23,'accepted','2026-02-10 17:23:16','2026-02-10 17:23:18','2026-02-10 20:24:16'),(88,42,1,23,'accepted','2026-02-10 23:21:03','2026-02-10 23:21:05','2026-02-11 02:22:03'),(89,43,23,1,'accepted','2026-02-10 23:36:31','2026-02-10 23:36:33','2026-02-11 02:37:31'),(90,44,1,23,'accepted','2026-02-11 02:46:27','2026-02-11 02:46:28','2026-02-11 05:47:27'),(91,45,1,23,'accepted','2026-02-11 02:52:45','2026-02-11 02:52:47','2026-02-11 05:53:45'),(94,NULL,1,23,'expired','2026-02-11 18:58:37',NULL,'2026-02-11 21:59:37'),(95,NULL,1,23,'expired','2026-02-11 18:58:55',NULL,'2026-02-11 21:59:55'),(96,NULL,1,23,'expired','2026-02-11 18:58:58',NULL,'2026-02-11 21:59:58'),(97,47,1,23,'accepted','2026-02-11 18:59:22','2026-02-11 18:59:26','2026-02-11 22:00:22'),(98,47,23,4,'expired','2026-02-11 19:05:56',NULL,'2026-02-11 22:06:56'),(99,48,1,23,'accepted','2026-02-11 21:40:51','2026-02-11 21:40:53','2026-02-12 00:41:51'),(100,49,1,23,'accepted','2026-02-11 21:42:27','2026-02-11 21:42:30','2026-02-12 00:43:27'),(101,50,1,23,'accepted','2026-02-11 22:52:21','2026-02-11 22:52:22','2026-02-12 01:53:21'),(104,53,1,23,'accepted','2026-02-12 14:04:29','2026-02-12 14:04:31','2026-02-12 17:05:29'),(105,54,1,23,'accepted','2026-02-12 20:15:07','2026-02-12 20:15:09','2026-02-12 23:16:07'),(106,55,1,23,'accepted','2026-02-12 21:29:51','2026-02-12 21:29:52','2026-02-13 00:30:51'),(107,56,23,1,'accepted','2026-02-12 23:12:29','2026-02-12 23:12:31','2026-02-13 02:13:29'),(108,57,1,23,'accepted','2026-02-12 23:31:06','2026-02-12 23:31:07','2026-02-13 02:32:06'),(109,58,23,1,'accepted','2026-02-12 23:43:21','2026-02-12 23:43:22','2026-02-13 02:44:21'),(110,59,1,23,'accepted','2026-02-12 23:50:56','2026-02-12 23:50:59','2026-02-13 02:51:56'),(111,60,1,23,'accepted','2026-02-13 00:15:33','2026-02-13 00:15:34','2026-02-13 03:16:33'),(112,61,14,25,'accepted','2026-02-13 02:28:40','2026-02-13 02:28:43','2026-02-13 05:29:40'),(113,62,1,23,'accepted','2026-02-13 02:33:19','2026-02-13 02:33:21','2026-02-13 05:34:19'),(114,63,1,23,'accepted','2026-02-13 15:54:07','2026-02-13 15:54:08','2026-02-13 18:55:07'),(115,64,1,23,'accepted','2026-02-13 16:55:02','2026-02-13 16:55:03','2026-02-13 19:56:02'),(116,65,23,1,'accepted','2026-02-14 10:13:43','2026-02-14 10:13:45','2026-02-14 13:14:43'),(117,66,1,23,'accepted','2026-02-15 00:14:47','2026-02-15 00:14:49','2026-02-15 03:15:47'),(118,67,1,23,'accepted','2026-02-19 13:23:54','2026-02-19 13:23:56','2026-02-19 16:24:54'),(119,68,1,23,'accepted','2026-02-19 13:54:03','2026-02-19 13:54:10','2026-02-19 16:55:03'),(120,69,1,23,'accepted','2026-02-19 13:55:32','2026-02-19 13:55:34','2026-02-19 16:56:32'),(121,70,1,23,'accepted','2026-02-19 15:13:45','2026-02-19 15:13:47','2026-02-19 18:14:45'),(123,72,1,23,'accepted','2026-02-20 17:09:44','2026-02-20 17:09:45','2026-02-20 20:10:44'),(124,73,23,1,'accepted','2026-02-20 19:47:44','2026-02-20 19:47:46','2026-02-20 22:48:44'),(126,75,23,1,'accepted','2026-02-22 15:20:14','2026-02-22 15:20:16','2026-02-22 18:21:14'),(127,76,1,32,'accepted','2026-02-24 15:11:31','2026-02-24 15:11:33','2026-02-24 18:12:31'),(129,78,1,34,'accepted','2026-02-25 00:35:26','2026-02-25 00:35:28','2026-02-25 03:36:26'),(131,80,1,23,'accepted','2026-02-25 19:46:55','2026-02-25 19:46:56','2026-02-25 22:47:55'),(132,81,23,1,'accepted','2026-02-26 03:24:58','2026-02-26 03:25:00','2026-02-26 06:25:58'),(133,82,1,23,'accepted','2026-02-26 13:25:30','2026-02-26 13:25:31','2026-02-26 16:26:30'),(138,NULL,33,25,'expired','2026-02-26 14:54:35',NULL,'2026-02-26 17:55:35'),(139,84,23,1,'accepted','2026-02-26 15:10:26','2026-02-26 15:10:27','2026-02-26 18:11:26'),(144,85,23,1,'accepted','2026-02-26 16:07:04','2026-02-26 16:07:06','2026-02-26 19:08:04'),(145,86,1,23,'accepted','2026-02-26 16:10:25','2026-02-26 16:10:28','2026-02-26 19:11:25'),(146,86,1,25,'accepted','2026-02-26 16:10:40','2026-02-26 16:10:43','2026-02-26 19:11:40'),(147,86,1,33,'accepted','2026-02-26 16:10:52','2026-02-26 16:10:54','2026-02-26 19:11:52'),(148,87,23,1,'accepted','2026-02-26 16:45:37','2026-02-26 16:45:40','2026-02-26 19:46:37'),(149,88,23,1,'accepted','2026-02-26 18:39:52','2026-02-26 18:39:54','2026-02-26 21:40:52'),(150,89,23,1,'accepted','2026-03-02 18:44:23','2026-03-02 18:44:24','2026-03-02 21:45:23'),(151,89,23,1,'expired','2026-03-02 18:47:34',NULL,'2026-03-02 21:48:34'),(152,90,23,1,'accepted','2026-03-04 23:00:11','2026-03-04 23:00:13','2026-03-05 02:01:11'),(153,91,1,25,'accepted','2026-03-06 01:14:44','2026-03-06 01:14:46','2026-03-06 04:15:44'),(154,92,1,25,'accepted','2026-03-10 15:26:56','2026-03-10 15:26:58','2026-03-10 18:27:56'),(155,93,1,25,'accepted','2026-03-11 22:51:30','2026-03-11 22:51:32','2026-03-12 01:52:30'),(156,NULL,23,25,'expired','2026-03-15 08:18:54',NULL,'2026-03-15 11:19:54'),(157,NULL,25,23,'expired','2026-03-15 08:18:59',NULL,'2026-03-15 11:19:59'),(158,NULL,23,25,'expired','2026-03-15 16:23:16',NULL,'2026-03-15 19:24:16'),(159,NULL,23,25,'expired','2026-03-15 16:24:13',NULL,'2026-03-15 19:25:13'),(160,NULL,23,25,'expired','2026-03-15 16:24:20',NULL,'2026-03-15 19:25:20'),(161,NULL,25,23,'expired','2026-03-15 16:55:58',NULL,'2026-03-15 19:56:58'),(162,NULL,23,25,'expired','2026-03-15 16:56:04',NULL,'2026-03-15 19:57:04'),(163,NULL,33,25,'expired','2026-03-16 13:35:14',NULL,'2026-03-16 16:36:14'),(164,NULL,23,25,'expired','2026-04-06 22:32:54',NULL,'2026-04-07 01:33:54'),(165,NULL,25,23,'expired','2026-04-06 23:09:39',NULL,'2026-04-07 02:10:39'),(166,NULL,23,25,'expired','2026-04-06 23:39:14',NULL,'2026-04-07 02:40:14'),(167,NULL,25,23,'expired','2026-04-07 00:29:26',NULL,'2026-04-07 03:30:26'),(168,NULL,23,25,'expired','2026-04-07 03:34:08',NULL,'2026-04-07 06:35:08'),(169,NULL,25,23,'expired','2026-04-07 03:34:37',NULL,'2026-04-07 06:35:37'),(170,94,25,23,'accepted','2026-04-07 03:48:15','2026-04-07 03:48:16','2026-04-07 06:49:15'),(171,95,23,25,'accepted','2026-04-07 20:49:39','2026-04-07 20:49:41','2026-04-07 23:50:39'),(172,96,23,25,'accepted','2026-04-08 23:07:45','2026-04-08 23:07:47','2026-04-09 02:08:45'),(173,97,23,25,'accepted','2026-04-09 19:17:48','2026-04-09 19:17:54','2026-04-09 22:18:48'),(174,98,23,1,'accepted','2026-04-09 20:21:13','2026-04-09 20:21:14','2026-04-09 23:22:13'),(175,99,23,25,'accepted','2026-04-09 20:24:08','2026-04-09 20:24:10','2026-04-09 23:25:08'),(176,100,23,25,'accepted','2026-04-12 22:05:07','2026-04-12 22:05:09','2026-04-13 01:06:07'),(177,101,23,25,'accepted','2026-04-14 02:01:27','2026-04-14 02:01:30','2026-04-14 05:02:27'),(178,102,25,23,'accepted','2026-04-17 16:48:31','2026-04-17 16:48:32','2026-04-17 19:49:31'),(179,103,23,25,'accepted','2026-04-17 21:54:51','2026-04-17 21:54:52','2026-04-18 00:55:51'),(180,104,25,23,'accepted','2026-04-19 06:50:14','2026-04-19 06:50:18','2026-04-19 09:51:14'),(181,NULL,25,33,'expired','2026-04-19 06:53:25',NULL,'2026-04-19 09:54:25'),(182,105,25,23,'accepted','2026-04-19 06:53:34','2026-04-19 06:53:36','2026-04-19 09:54:34'),(183,106,2,23,'accepted','2026-04-27 20:07:36','2026-04-27 20:07:38','2026-04-27 23:08:36'),(184,107,1,23,'accepted','2026-05-06 15:31:33','2026-05-06 15:31:35','2026-05-06 18:32:33'),(185,108,1,46,'accepted','2026-05-06 16:10:35','2026-05-06 16:10:40','2026-05-06 19:11:35'),(186,109,2,23,'accepted','2026-05-09 00:04:00','2026-05-09 00:04:01','2026-05-09 03:05:00'),(187,110,23,2,'accepted','2026-05-10 02:57:18','2026-05-10 02:57:22','2026-05-10 05:58:18'),(188,111,2,23,'accepted','2026-05-10 03:09:29','2026-05-10 03:09:31','2026-05-10 06:10:29'),(189,112,25,23,'accepted','2026-05-10 03:21:23','2026-05-10 03:21:25','2026-05-10 06:22:23'),(190,113,23,25,'accepted','2026-05-10 05:52:00','2026-05-10 05:52:03','2026-05-10 08:53:00'),(191,114,25,27,'accepted','2026-05-10 13:31:47','2026-05-10 13:31:48','2026-05-10 16:32:47'),(192,115,25,27,'accepted','2026-05-10 13:34:28','2026-05-10 13:34:31','2026-05-10 16:35:28'),(193,116,23,25,'accepted','2026-05-10 17:05:24','2026-05-10 17:05:26','2026-05-10 20:06:24'),(194,117,23,1,'accepted','2026-05-10 19:18:37','2026-05-10 19:18:39','2026-05-10 22:19:37'),(195,118,23,25,'accepted','2026-05-10 21:15:36','2026-05-10 21:15:38','2026-05-11 00:16:36'),(196,119,27,23,'accepted','2026-05-11 01:19:30','2026-05-11 01:19:32','2026-05-11 04:20:30'),(197,120,27,23,'accepted','2026-05-11 01:52:29','2026-05-11 01:52:31','2026-05-11 04:53:29'),(198,121,27,23,'accepted','2026-05-11 02:12:28','2026-05-11 02:12:37','2026-05-11 05:13:28'),(199,NULL,1,23,'expired','2026-05-15 23:59:56',NULL,'2026-05-16 03:00:56'),(200,NULL,1,23,'expired','2026-05-16 00:00:09',NULL,'2026-05-16 03:01:09'),(201,NULL,1,23,'expired','2026-05-16 00:01:49',NULL,'2026-05-16 03:02:49'),(202,NULL,32,1,'expired','2026-05-16 00:09:45',NULL,'2026-05-16 03:10:45'),(203,122,23,27,'accepted','2026-05-16 00:41:28','2026-05-16 00:41:30','2026-05-16 03:42:28'),(204,NULL,23,27,'expired','2026-05-17 00:06:07',NULL,'2026-05-17 03:07:07'),(205,123,1,23,'accepted','2026-05-17 00:08:12','2026-05-17 00:08:14','2026-05-17 03:09:12'),(206,124,25,1,'accepted','2026-05-21 18:16:26','2026-05-21 18:16:28','2026-05-21 21:17:26'),(207,125,1,32,'accepted','2026-05-21 20:02:49','2026-05-21 20:02:51','2026-05-21 23:03:49'),(208,NULL,1,32,'expired','2026-05-21 20:03:39',NULL,'2026-05-21 23:04:39'),(209,127,32,1,'accepted','2026-05-21 20:03:46','2026-05-21 20:03:48','2026-05-21 23:04:46'),(210,128,25,1,'accepted','2026-05-21 20:12:28','2026-05-21 20:12:30','2026-05-21 23:13:28'),(211,129,23,1,'accepted','2026-05-21 20:15:48','2026-05-21 20:15:50','2026-05-21 23:16:48'),(212,130,1,25,'accepted','2026-05-21 20:39:06','2026-05-21 20:39:08','2026-05-21 23:40:06'),(213,131,1,23,'accepted','2026-05-21 21:19:54','2026-05-21 21:19:56','2026-05-22 00:20:54'),(214,131,1,23,'expired','2026-05-21 21:20:03',NULL,'2026-05-22 00:21:03'),(215,131,23,1,'expired','2026-05-21 21:20:09',NULL,'2026-05-22 00:21:09'),(216,131,1,23,'expired','2026-05-21 21:20:20',NULL,'2026-05-22 00:21:20'),(217,132,23,1,'accepted','2026-05-21 21:22:24','2026-05-21 21:22:25','2026-05-22 00:23:24'),(218,133,1,25,'accepted','2026-05-21 21:33:54','2026-05-21 21:33:57','2026-05-22 00:34:54'),(219,134,1,25,'accepted','2026-05-21 22:36:29','2026-05-21 22:36:30','2026-05-22 01:37:29'),(220,135,1,25,'accepted','2026-05-21 22:51:34','2026-05-21 22:51:35','2026-05-22 01:52:34'),(221,136,1,23,'accepted','2026-05-22 00:10:27','2026-05-22 00:10:29','2026-05-22 03:11:27'),(222,137,1,25,'accepted','2026-05-23 01:04:18','2026-05-23 01:04:20','2026-05-23 04:05:18'),(223,138,25,1,'accepted','2026-05-24 19:45:52','2026-05-24 19:45:53','2026-05-24 22:46:52'),(224,139,1,25,'accepted','2026-05-24 23:03:38','2026-05-24 23:03:39','2026-05-25 02:04:38'),(225,140,25,1,'accepted','2026-05-30 17:55:36','2026-05-30 17:55:42','2026-05-30 20:56:36'),(226,NULL,25,1,'expired','2026-05-31 03:16:29',NULL,'2026-05-31 06:17:29'),(227,NULL,25,1,'expired','2026-05-31 03:16:44',NULL,'2026-05-31 06:17:44'),(228,141,1,25,'accepted','2026-05-31 03:17:17','2026-05-31 03:17:22','2026-05-31 06:18:17'),(229,NULL,25,1,'expired','2026-05-31 03:46:03',NULL,'2026-05-31 06:47:03'),(230,NULL,25,1,'expired','2026-05-31 04:46:10',NULL,'2026-05-31 07:47:10'),(231,NULL,25,1,'expired','2026-05-31 13:03:29',NULL,'2026-05-31 16:04:29'),(232,142,1,23,'accepted','2026-05-31 15:42:10','2026-05-31 15:42:12','2026-05-31 18:43:10'),(233,143,23,1,'accepted','2026-06-19 02:03:55','2026-06-19 02:03:57','2026-06-19 05:04:55'),(234,144,1,23,'accepted','2026-06-20 19:58:41','2026-06-20 19:58:43','2026-06-20 22:59:41'),(235,NULL,1,23,'expired','2026-06-20 22:47:05',NULL,'2026-06-21 01:48:05'),(236,NULL,23,1,'expired','2026-06-20 22:47:10',NULL,'2026-06-21 01:48:10'),(237,NULL,23,1,'expired','2026-06-20 22:47:11',NULL,'2026-06-21 01:48:11'),(238,NULL,1,23,'expired','2026-06-20 22:48:35',NULL,'2026-06-21 01:49:35'),(239,NULL,23,1,'expired','2026-06-20 22:48:42',NULL,'2026-06-21 01:49:42'),(240,NULL,1,23,'expired','2026-06-20 23:20:04',NULL,'2026-06-21 02:21:04'),(241,NULL,23,1,'expired','2026-06-20 23:20:11',NULL,'2026-06-21 02:21:11'),(242,NULL,1,23,'expired','2026-06-20 23:20:28',NULL,'2026-06-21 02:21:28'),(243,NULL,1,23,'expired','2026-06-20 23:49:26',NULL,'2026-06-21 02:50:26'),(244,NULL,23,1,'expired','2026-06-20 23:49:30',NULL,'2026-06-21 02:50:30'),(245,NULL,23,1,'expired','2026-06-20 23:49:32',NULL,'2026-06-21 02:50:32'),(246,NULL,1,23,'expired','2026-06-20 23:49:35',NULL,'2026-06-21 02:50:35'),(247,NULL,23,1,'expired','2026-06-20 23:50:26',NULL,'2026-06-21 02:51:26'),(248,145,23,1,'accepted','2026-06-21 00:20:03','2026-06-21 00:20:04','2026-06-21 03:21:03'),(249,146,1,23,'accepted','2026-06-21 00:24:46','2026-06-21 00:24:48','2026-06-21 03:25:46'),(250,147,23,1,'accepted','2026-06-21 00:41:13','2026-06-21 00:41:14','2026-06-21 03:42:13'),(251,148,1,23,'accepted','2026-06-21 01:07:54','2026-06-21 01:07:55','2026-06-21 04:08:54'),(252,148,23,25,'accepted','2026-06-21 01:08:00','2026-06-21 01:08:02','2026-06-21 04:09:00'),(253,149,1,23,'accepted','2026-06-23 02:10:17','2026-06-23 02:10:19','2026-06-23 05:11:17'),(254,150,25,1,'accepted','2026-06-24 03:42:35','2026-06-24 03:42:38','2026-06-24 06:43:35'),(255,151,1,25,'accepted','2026-06-24 04:35:39','2026-06-24 04:35:41','2026-06-24 07:36:39'),(256,152,1,25,'accepted','2026-06-24 14:28:08','2026-06-24 14:28:10','2026-06-24 17:29:08'),(257,153,25,1,'accepted','2026-06-24 15:35:33','2026-06-24 15:35:34','2026-06-24 18:36:33'),(258,154,1,23,'accepted','2026-06-24 18:19:36','2026-06-24 18:19:37','2026-06-24 21:20:36'),(259,155,25,1,'accepted','2026-06-25 01:37:52','2026-06-25 01:37:53','2026-06-25 04:38:52'),(260,156,25,1,'accepted','2026-06-25 02:42:07','2026-06-25 02:42:08','2026-06-25 05:43:07'),(261,157,1,25,'accepted','2026-06-25 03:19:44','2026-06-25 03:19:51','2026-06-25 06:20:44'),(262,158,25,1,'accepted','2026-06-25 05:13:01','2026-06-25 05:13:03','2026-06-25 08:14:01'),(263,159,25,1,'accepted','2026-06-25 14:30:36','2026-06-25 14:30:38','2026-06-25 17:31:36'),(264,160,1,32,'accepted','2026-06-27 13:58:17','2026-06-27 13:58:18','2026-06-27 16:59:17'),(265,161,32,1,'accepted','2026-06-28 06:32:42','2026-06-28 06:32:43','2026-06-28 09:33:42'),(266,162,32,1,'accepted','2026-07-02 18:16:34','2026-07-02 18:16:35','2026-07-02 21:17:34');
/*!40000 ALTER TABLE `party_invites` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `party_members`
--

DROP TABLE IF EXISTS `party_members`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `party_members` (
  `party_id` int unsigned NOT NULL,
  `player_id` bigint unsigned NOT NULL,
  `joined_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`party_id`,`player_id`),
  KEY `idx_player` (`player_id`),
  CONSTRAINT `party_members_ibfk_1` FOREIGN KEY (`party_id`) REFERENCES `parties` (`party_id`) ON DELETE CASCADE,
  CONSTRAINT `party_members_ibfk_2` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `party_members`
--

LOCK TABLES `party_members` WRITE;
/*!40000 ALTER TABLE `party_members` DISABLE KEYS */;
INSERT INTO `party_members` VALUES (4,30,'2026-02-09 01:15:02');
/*!40000 ALTER TABLE `party_members` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `personal_shop_listings`
--

DROP TABLE IF EXISTS `personal_shop_listings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `personal_shop_listings` (
  `listing_id` int unsigned NOT NULL AUTO_INCREMENT,
  `shop_id` int unsigned NOT NULL,
  `slot_index` tinyint unsigned NOT NULL COMMENT '0-9',
  `inventory_id` int NOT NULL,
  `price_gold` bigint unsigned NOT NULL,
  `status` enum('listed','sold','cancelled') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'listed',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`listing_id`),
  UNIQUE KEY `uk_shop_slot` (`shop_id`,`slot_index`),
  KEY `idx_shop_listing` (`shop_id`,`status`),
  KEY `idx_inventory` (`inventory_id`),
  CONSTRAINT `personal_shop_listings_ibfk_1` FOREIGN KEY (`shop_id`) REFERENCES `personal_shops` (`shop_id`) ON DELETE CASCADE,
  CONSTRAINT `personal_shop_listings_ibfk_2` FOREIGN KEY (`inventory_id`) REFERENCES `player_inventory` (`inventory_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=73 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `personal_shop_listings`
--

LOCK TABLES `personal_shop_listings` WRITE;
/*!40000 ALTER TABLE `personal_shop_listings` DISABLE KEYS */;
INSERT INTO `personal_shop_listings` VALUES (7,4,7,225,10000,'cancelled','2026-04-12 14:59:09'),(8,5,0,199,20000,'cancelled','2026-04-12 16:41:05'),(9,6,0,199,50000,'cancelled','2026-04-12 16:47:16'),(10,7,0,199,50000,'cancelled','2026-04-12 16:48:29'),(11,8,0,199,999999,'cancelled','2026-04-12 16:58:08'),(12,9,0,197,555555555,'cancelled','2026-04-12 20:29:45'),(13,10,0,199,11111,'cancelled','2026-04-12 20:35:46'),(14,11,0,199,111111,'cancelled','2026-04-12 20:45:03'),(15,12,0,206,555555,'cancelled','2026-04-12 22:06:02'),(16,13,0,199,999999,'cancelled','2026-04-12 22:26:56'),(17,14,0,206,222222,'cancelled','2026-04-12 22:59:36'),(18,15,0,206,5,'cancelled','2026-04-12 23:29:10'),(19,16,0,206,11111,'cancelled','2026-04-12 23:35:03'),(20,17,0,197,40000,'cancelled','2026-04-13 00:25:37'),(21,18,0,197,40000,'cancelled','2026-04-13 00:28:08'),(22,18,1,206,25000,'cancelled','2026-04-13 00:28:08'),(23,18,2,199,4444,'cancelled','2026-04-13 00:28:08'),(26,20,0,228,2000,'sold','2026-04-13 00:35:36'),(27,21,0,206,50000,'cancelled','2026-04-13 00:39:08'),(29,23,0,199,111,'sold','2026-04-13 01:15:02'),(36,29,1,199,50,'sold','2026-04-13 03:49:49'),(41,34,0,228,1,'sold','2026-04-13 13:25:15'),(44,37,0,199,1111,'cancelled','2026-04-13 23:44:20'),(45,38,0,199,1111,'cancelled','2026-04-14 01:19:51'),(47,40,0,94,55555,'cancelled','2026-04-14 18:58:20'),(49,42,0,106,222222,'cancelled','2026-04-17 00:18:57'),(52,45,0,97,9999999,'cancelled','2026-05-06 16:12:47'),(53,46,0,97,99999999,'cancelled','2026-05-06 16:21:58'),(54,47,0,97,99999999,'cancelled','2026-05-06 16:24:00'),(55,48,0,239,5000,'sold','2026-05-06 16:25:34'),(58,50,1,106,999999,'cancelled','2026-05-09 00:02:03'),(59,51,0,199,99999,'cancelled','2026-05-09 00:08:01'),(60,52,0,199,99,'sold','2026-05-09 00:09:26'),(62,54,0,237,999999,'cancelled','2026-05-10 04:45:23'),(63,55,0,228,1111,'sold','2026-05-10 04:58:09'),(67,59,0,218,111,'sold','2026-05-10 21:14:02'),(71,63,0,96,99,'cancelled','2026-06-23 02:13:59'),(72,64,0,96,99,'sold','2026-06-23 02:14:58');
/*!40000 ALTER TABLE `personal_shop_listings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `personal_shops`
--

DROP TABLE IF EXISTS `personal_shops`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `personal_shops` (
  `shop_id` int unsigned NOT NULL AUTO_INCREMENT,
  `seller_player_id` bigint unsigned NOT NULL,
  `shop_name` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `status` enum('open','closed') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'open',
  `opened_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `closed_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`shop_id`),
  KEY `idx_seller` (`seller_player_id`),
  KEY `idx_seller_open` (`seller_player_id`,`status`),
  CONSTRAINT `personal_shops_ibfk_1` FOREIGN KEY (`seller_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=65 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `personal_shops`
--

LOCK TABLES `personal_shops` WRITE;
/*!40000 ALTER TABLE `personal_shops` DISABLE KEYS */;
INSERT INTO `personal_shops` VALUES (1,1,'jeffo','closed','2026-04-12 04:49:25','2026-04-12 04:49:30'),(2,1,'Jeffo vendas','closed','2026-04-12 12:56:03','2026-04-12 13:46:31'),(3,1,'a','closed','2026-04-12 13:46:31','2026-04-12 13:46:47'),(4,1,'vendas','closed','2026-04-12 14:59:09','2026-04-12 14:59:35'),(5,23,'jeffo store','closed','2026-04-12 16:41:05','2026-04-12 16:41:49'),(6,23,'aaa','closed','2026-04-12 16:47:16','2026-04-12 16:48:14'),(7,23,'jeffito vendas','closed','2026-04-12 16:48:29','2026-04-12 16:58:08'),(8,23,'thekillzone store','closed','2026-04-12 16:58:08','2026-04-12 20:29:45'),(9,23,'jewf','closed','2026-04-12 20:29:45','2026-04-12 20:30:12'),(10,23,'aaaaaa','closed','2026-04-12 20:35:46','2026-04-12 20:45:03'),(11,23,'VENDAS','closed','2026-04-12 20:45:03','2026-04-12 22:06:02'),(12,23,'jeffo vendas','closed','2026-04-12 22:06:02','2026-04-12 22:26:56'),(13,23,'teste loja','closed','2026-04-12 22:26:56','2026-04-12 22:59:36'),(14,23,'sttt','closed','2026-04-12 22:59:36','2026-04-12 23:29:10'),(15,23,'test','closed','2026-04-12 23:29:10','2026-04-12 23:35:03'),(16,23,'test','closed','2026-04-12 23:35:03','2026-04-13 00:25:37'),(17,23,'test shop','closed','2026-04-13 00:25:37','2026-04-13 00:27:28'),(18,23,'test shop','closed','2026-04-13 00:28:08','2026-04-13 00:33:55'),(19,23,'test','closed','2026-04-13 00:33:55','2026-04-13 00:34:22'),(20,23,'test','closed','2026-04-13 00:35:36','2026-04-13 00:39:08'),(21,23,'test','closed','2026-04-13 00:39:08','2026-04-13 01:15:02'),(22,25,'test','closed','2026-04-13 01:13:08','2026-04-13 01:15:09'),(23,23,'jeff','closed','2026-04-13 01:15:02','2026-04-13 01:15:52'),(24,23,'jeff','closed','2026-04-13 01:16:25','2026-04-13 01:17:18'),(25,23,'jeff','closed','2026-04-13 01:17:43','2026-04-13 02:18:49'),(26,23,'aaa','closed','2026-04-13 02:18:49','2026-04-13 02:58:35'),(27,23,'aa','closed','2026-04-13 02:58:35','2026-04-13 13:24:43'),(28,25,'jeffo store','closed','2026-04-13 03:48:37','2026-04-13 03:49:33'),(29,25,'jeffo store','closed','2026-04-13 03:49:48','2026-04-13 03:50:31'),(30,25,'aa','closed','2026-04-13 12:35:06','2026-04-13 12:36:11'),(31,25,'aaaaaa','closed','2026-04-13 12:36:46','2026-04-13 12:37:21'),(32,25,'aaaa','closed','2026-04-13 13:02:51','2026-04-13 13:25:15'),(33,23,'aaa','closed','2026-04-13 13:24:43','2026-04-13 13:24:47'),(34,25,'vendas','closed','2026-04-13 13:25:15','2026-04-13 13:25:38'),(35,25,'aaa','closed','2026-04-13 13:25:57','2026-04-13 13:28:15'),(36,23,'jeffo venda','closed','2026-04-13 17:39:50','2026-04-13 23:44:20'),(37,23,'loja','closed','2026-04-13 23:44:20','2026-04-13 23:44:27'),(38,23,'jeffo store','closed','2026-04-14 01:19:51','2026-04-14 01:20:08'),(39,25,'vendas','closed','2026-04-14 01:58:31','2026-04-14 01:58:54'),(40,1,'selling','closed','2026-04-14 18:58:20','2026-04-14 18:58:32'),(41,23,'vendas','closed','2026-04-15 20:43:54','2026-04-15 20:44:15'),(42,23,'aaaa','closed','2026-04-17 00:18:57','2026-04-17 00:19:09'),(43,25,'shop','closed','2026-04-17 18:38:27','2026-04-17 18:38:41'),(44,25,'vendo pedra','closed','2026-04-19 06:42:27','2026-04-19 06:43:14'),(45,1,'vendas','closed','2026-05-06 16:12:47','2026-05-06 16:21:58'),(46,1,'aaa','closed','2026-05-06 16:21:58','2026-05-06 16:23:56'),(47,1,'aaa','closed','2026-05-06 16:24:00','2026-05-06 16:24:12'),(48,1,'espada','closed','2026-05-06 16:25:34','2026-05-06 16:26:01'),(49,1,'jeffo vendas','closed','2026-05-08 23:59:01','2026-05-08 23:59:28'),(50,23,'itens','closed','2026-05-09 00:02:03','2026-05-09 00:02:45'),(51,23,'aoaoa','closed','2026-05-09 00:08:01','2026-05-09 00:08:12'),(52,23,'aoaoa','closed','2026-05-09 00:09:26','2026-05-09 00:10:34'),(53,1,'jeffo store','closed','2026-05-10 04:26:41','2026-05-10 04:45:23'),(54,1,'jefffo','closed','2026-05-10 04:45:23','2026-05-10 04:46:33'),(55,27,'aaaa','closed','2026-05-10 04:58:09','2026-05-10 04:58:53'),(56,23,'jeffo','closed','2026-05-10 05:49:33','2026-05-10 05:50:27'),(57,25,'loja','closed','2026-05-10 13:33:29','2026-05-10 13:34:20'),(58,23,'venda','closed','2026-05-10 19:00:20','2026-05-10 19:01:37'),(59,23,'jeffo','closed','2026-05-10 21:14:02','2026-05-10 21:14:33'),(60,1,'stt','closed','2026-05-22 00:25:46','2026-05-22 00:26:42'),(61,1,'jeffo store','closed','2026-06-18 19:51:32','2026-06-18 19:51:42'),(62,1,'jeffo store','closed','2026-06-21 21:32:40','2026-06-21 21:32:55'),(63,1,'aa','closed','2026-06-23 02:13:59','2026-06-23 02:14:56'),(64,1,'aa','closed','2026-06-23 02:14:58','2026-06-23 02:15:36');
/*!40000 ALTER TABLE `personal_shops` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_consumable_cooldowns`
--

DROP TABLE IF EXISTS `player_consumable_cooldowns`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_consumable_cooldowns` (
  `player_id` bigint unsigned NOT NULL,
  `item_subtype` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `expires_at_ms` bigint unsigned NOT NULL,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`,`item_subtype`),
  CONSTRAINT `fk_pcc_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_consumable_cooldowns`
--

LOCK TABLES `player_consumable_cooldowns` WRITE;
/*!40000 ALTER TABLE `player_consumable_cooldowns` DISABLE KEYS */;
INSERT INTO `player_consumable_cooldowns` VALUES (1,'buff_potion',1783141039371,'2026-07-04 04:57:18'),(1,'health_potion',1783127859800,'2026-07-04 01:17:34'),(1,'mana_potion',1783097597214,'2026-07-03 16:53:12'),(23,'buff_potion',1783114333479,'2026-07-03 21:32:12'),(23,'health_potion',1779408771938,'2026-05-22 00:12:46'),(25,'buff_potion',1783177489117,'2026-07-04 15:04:48'),(25,'health_potion',1782009197394,'2026-06-21 02:33:12'),(32,'buff_potion',1783127782784,'2026-07-04 01:16:21'),(32,'health_potion',1783127859818,'2026-07-04 01:17:34');
/*!40000 ALTER TABLE `player_consumable_cooldowns` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_cooldowns`
--

DROP TABLE IF EXISTS `player_cooldowns`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_cooldowns` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `skill_id` int unsigned NOT NULL,
  `started_at` timestamp(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  `expires_at` timestamp(3) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_player_cooldown` (`player_id`,`skill_id`),
  KEY `idx_expires` (`expires_at`),
  KEY `fk_cooldown_skill` (`skill_id`),
  CONSTRAINT `fk_cooldown_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_cooldown_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=384 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_cooldowns`
--

LOCK TABLES `player_cooldowns` WRITE;
/*!40000 ALTER TABLE `player_cooldowns` DISABLE KEYS */;
INSERT INTO `player_cooldowns` VALUES (1,1,1,'2026-05-23 01:04:53.045','2026-05-23 04:04:56.000'),(3,1,9,'2026-05-20 18:40:58.624','2026-05-20 21:41:43.000'),(4,1,7,'2026-06-19 13:44:39.902','2026-06-19 16:44:45.000'),(15,1,15,'2026-05-20 22:26:58.239','2026-05-21 01:29:58.000'),(29,1,13,'2026-05-22 00:30:00.031','2026-05-22 03:30:12.000'),(31,32,31,'2026-05-21 20:04:39.423','2026-05-21 23:04:42.000'),(52,1,3,'2026-05-21 21:34:59.390','2026-05-22 00:35:07.000'),(61,1,10,'2026-05-21 23:09:42.317','2026-05-22 02:09:52.000'),(236,1,5,'2026-05-31 19:47:39.699','2026-05-31 22:48:09.000'),(383,31,61,'2026-06-19 19:42:10.057','2026-06-19 22:42:16.000');
/*!40000 ALTER TABLE `player_cooldowns` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_inventory`
--

DROP TABLE IF EXISTS `player_inventory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_inventory` (
  `inventory_id` int NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `item_template_id` int NOT NULL,
  `quantity` int NOT NULL DEFAULT '1' COMMENT 'Quantidade do item (para itens empilháveis)',
  `slot_index` int NOT NULL COMMENT 'Índice do slot no inventário (0-49 para inventário de 50 slots)',
  `auction_listing_id` int unsigned DEFAULT NULL COMMENT 'Anúncio ativo do mercado; NULL = item visível na bolsa',
  `is_equipped` tinyint(1) DEFAULT '0' COMMENT 'Se o item está equipado',
  `durability` float DEFAULT '100' COMMENT 'Durabilidade do item (100 = novo, 0 = quebrado)',
  `custom_properties` json DEFAULT NULL COMMENT 'Propriedades únicas desta instância: {"enchantment": "fire", "bonus_stats": {}, etc.}',
  `acquired_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Quando o item foi adquirido',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `refinement_level` tinyint unsigned DEFAULT '0' COMMENT 'Nível de refinação (+0 a +12)',
  `refinement_bonus_stats` json DEFAULT NULL COMMENT 'Stats bônus calculados da refinação',
  PRIMARY KEY (`inventory_id`),
  UNIQUE KEY `unique_player_slot` (`player_id`,`slot_index`),
  KEY `item_template_id` (`item_template_id`),
  KEY `idx_player_inventory` (`player_id`),
  KEY `idx_slot_index` (`player_id`,`slot_index`),
  KEY `idx_equipped` (`player_id`,`is_equipped`),
  KEY `idx_player_inventory_auction` (`auction_listing_id`),
  KEY `idx_refinement_level` (`refinement_level`),
  CONSTRAINT `player_inventory_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `player_inventory_ibfk_2` FOREIGN KEY (`item_template_id`) REFERENCES `item_templates` (`item_id`) ON DELETE RESTRICT
) ENGINE=InnoDB AUTO_INCREMENT=353 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_inventory`
--

LOCK TABLES `player_inventory` WRITE;
/*!40000 ALTER TABLE `player_inventory` DISABLE KEYS */;
INSERT INTO `player_inventory` VALUES (1,27,1,1,-1,NULL,1,100,NULL,'2025-11-14 17:47:15','2026-06-22 02:50:12',0,NULL),(4,27,4,1,-4,NULL,1,100,NULL,'2025-11-14 17:47:15','2026-05-10 05:15:13',0,NULL),(5,27,5,1,-5,NULL,1,100,NULL,'2025-11-14 17:47:15','2026-04-12 15:12:26',0,NULL),(65,1,35,1,-65,NULL,1,100,'[]','2025-11-28 20:08:53','2026-07-04 01:14:58',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(66,1,36,1,-66,NULL,1,100,'[]','2025-11-28 20:08:53','2026-07-04 01:15:00',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(80,1,37,1,-80,NULL,1,100,'[]','2025-11-28 21:17:41','2026-07-04 01:15:05',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(81,1,38,1,-81,NULL,1,100,'[]','2025-11-28 21:17:41','2026-07-04 01:15:02',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(82,27,6,1,-82,NULL,1,100,'[]','2025-11-28 21:17:41','2025-12-18 16:35:39',0,NULL),(91,23,39,1,12,NULL,0,100,NULL,'2025-11-30 13:12:39','2026-07-03 21:32:29',3,'{\"defense\": 1, \"health_bonus\": 2, \"magic_defense\": 1}'),(93,23,1,1,-93,NULL,1,100,NULL,'2025-12-01 02:53:58','2026-02-08 19:14:32',0,NULL),(94,46,40,1,-94,NULL,1,100,NULL,'2025-12-01 05:07:15','2026-05-06 16:27:14',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 11, \"defense\": 11, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 11, \"magic_defense\": 11, \"double_attack_rate\": 3}'),(95,23,4,1,-95,NULL,1,100,NULL,'2025-12-01 05:16:33','2026-05-10 20:25:46',0,NULL),(96,1,41,1,-96,NULL,1,100,NULL,'2025-12-01 05:16:40','2026-07-04 01:14:56',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 6, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 6, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(97,1,42,1,-97,NULL,1,100,NULL,'2025-12-01 05:25:16','2026-07-04 01:14:50',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 3, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 3, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(98,1,44,1,-98,NULL,1,100,NULL,'2025-12-01 17:39:37','2026-07-04 01:14:52',12,'{\"luck\": 6, \"dodge\": 6, \"attack\": 1, \"defense\": 69, \"accuracy\": 1, \"critical\": 1, \"movement\": 1, \"strength\": 6, \"vitality\": 6, \"dexterity\": 6, \"mana_bonus\": 110, \"resistance\": 1, \"health_bonus\": 110, \"intelligence\": 6, \"magic_attack\": 1, \"magic_defense\": 54, \"double_attack_rate\": 1}'),(99,1,45,1,-99,NULL,1,100,NULL,'2025-12-01 17:48:06','2026-07-04 01:14:54',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 3, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 3, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(100,1,46,1,-100,NULL,1,100,NULL,'2025-12-01 17:57:30','2026-07-04 01:15:06',12,'{\"attack\": 96, \"accuracy\": 12, \"critical\": 7, \"strength\": 6, \"dexterity\": 3, \"health_bonus\": 41, \"magic_attack\": 26, \"double_attack_rate\": 3}'),(101,27,47,1,-101,NULL,1,100,NULL,'2025-12-01 23:23:50','2026-06-22 02:50:16',0,NULL),(102,27,48,1,-102,NULL,1,100,NULL,'2025-12-02 00:29:40','2026-06-22 02:50:22',0,NULL),(103,27,49,1,-103,NULL,1,100,NULL,'2025-12-02 02:21:58','2026-06-22 02:50:19',0,NULL),(105,27,50,1,-105,NULL,1,100,NULL,'2025-12-02 12:04:08','2026-07-05 01:19:41',0,NULL),(106,23,51,1,11,NULL,0,100,NULL,'2025-12-02 16:55:48','2026-06-23 02:17:34',3,'{\"defense\": 8, \"dexterity\": 1, \"mana_bonus\": 8, \"health_bonus\": 8, \"magic_defense\": 9}'),(181,23,16,44,3,NULL,0,100,NULL,'2026-02-08 15:51:58','2026-07-03 21:32:32',0,NULL),(188,23,16,1,65,NULL,0,100,NULL,'2026-02-08 20:23:05','2026-05-10 20:09:07',0,NULL),(193,4,11,1,0,NULL,0,100,NULL,'2026-02-09 17:00:10','2026-02-09 17:00:22',0,NULL),(197,23,11,2,2,NULL,0,100,NULL,'2026-02-09 23:15:28','2026-07-03 21:32:12',0,NULL),(199,2,16,1,0,NULL,0,100,NULL,'2026-02-11 00:09:30','2026-05-09 00:10:29',0,NULL),(202,23,11,1,4,NULL,0,100,NULL,'2026-02-11 21:44:09','2026-02-14 10:15:55',0,NULL),(206,23,9,2,7,NULL,0,100,NULL,'2026-02-13 16:59:21','2026-02-13 17:03:16',0,NULL),(213,34,11,1,0,NULL,0,100,NULL,'2026-02-25 00:36:04','2026-02-25 00:36:11',0,NULL),(218,25,16,43,0,NULL,0,100,NULL,'2026-03-10 15:30:08','2026-05-21 18:14:45',0,NULL),(225,23,13,2,9,NULL,0,100,NULL,'2026-04-11 14:31:05','2026-04-15 20:43:29',0,NULL),(228,27,7,1,0,NULL,0,100,NULL,'2026-04-13 00:35:15','2026-05-11 01:22:46',0,NULL),(237,1,52,592,2,NULL,0,100,NULL,'2026-04-28 03:03:52','2026-07-03 03:57:42',0,NULL),(239,46,1,1,-239,NULL,1,100,NULL,'2026-04-28 03:07:08','2026-05-06 16:25:53',12,'{\"attack\": 55, \"accuracy\": 7, \"critical\": 3, \"strength\": 3, \"dexterity\": 3, \"health_bonus\": 28, \"magic_attack\": 14, \"double_attack_rate\": 3}'),(240,23,4,1,8,NULL,0,100,NULL,'2026-04-28 03:07:26','2026-07-03 21:32:27',12,'{\"defense\": 11, \"movement\": 14, \"strength\": 3, \"vitality\": 3, \"resistance\": 3, \"health_bonus\": 28, \"magic_defense\": 8}'),(242,23,52,43,10,NULL,0,100,NULL,'2026-05-10 18:54:29','2026-06-23 02:16:18',0,NULL),(248,1,53,334,6,NULL,0,100,NULL,'2026-05-17 03:21:05','2026-07-04 01:05:28',0,NULL),(250,1,52,109,1,NULL,0,100,NULL,'2026-05-17 03:21:13','2026-07-03 03:05:45',0,NULL),(261,1,11,51,16,NULL,0,100,NULL,'2026-05-21 16:15:34','2026-07-04 01:04:10',0,NULL),(262,1,12,5,18,NULL,0,100,NULL,'2026-05-21 16:15:39','2026-07-04 01:04:13',0,NULL),(263,1,13,32,17,NULL,0,100,NULL,'2026-05-21 16:15:44','2026-07-04 01:04:11',0,NULL),(264,1,14,14,19,NULL,0,100,NULL,'2026-05-21 16:15:49','2026-07-04 01:04:14',0,NULL),(265,25,10,5,2,NULL,0,100,NULL,'2026-05-21 20:39:23','2026-07-03 22:53:27',0,NULL),(266,25,11,3,3,NULL,0,100,NULL,'2026-05-21 20:39:27','2026-07-03 22:53:30',0,NULL),(267,25,13,6,4,NULL,0,100,NULL,'2026-05-21 20:39:31','2026-07-03 22:53:31',0,NULL),(269,25,14,1,6,NULL,0,100,NULL,'2026-05-21 20:39:42','2026-07-03 22:53:33',0,NULL),(275,32,41,1,-275,NULL,1,100,NULL,'2026-07-01 04:52:40','2026-07-01 05:26:31',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 6, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 6, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(276,32,36,1,-276,NULL,1,100,NULL,'2026-07-01 04:52:47','2026-07-01 18:51:07',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(277,32,40,1,1,NULL,0,100,NULL,'2026-07-01 04:53:25','2026-07-02 18:23:15',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 11, \"defense\": 11, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 11, \"magic_defense\": 11, \"double_attack_rate\": 3}'),(278,32,42,1,-278,NULL,1,100,NULL,'2026-07-01 04:53:30','2026-07-01 05:26:27',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 3, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 3, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(279,32,45,1,-279,NULL,1,100,NULL,'2026-07-01 04:53:34','2026-07-01 05:29:22',12,'{\"luck\": 3, \"dodge\": 3, \"attack\": 3, \"defense\": 55, \"accuracy\": 3, \"critical\": 3, \"movement\": 3, \"strength\": 3, \"vitality\": 3, \"dexterity\": 3, \"mana_bonus\": 28, \"resistance\": 3, \"health_bonus\": 28, \"intelligence\": 3, \"magic_attack\": 3, \"magic_defense\": 55, \"double_attack_rate\": 3}'),(280,32,35,1,-280,NULL,1,100,NULL,'2026-07-01 04:54:01','2026-07-01 05:26:20',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(281,32,37,1,-281,NULL,1,100,NULL,'2026-07-01 04:54:06','2026-07-01 05:26:22',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(282,32,44,1,-282,NULL,1,100,NULL,'2026-07-01 04:54:37','2026-07-01 05:27:54',12,'{\"luck\": 6, \"dodge\": 6, \"attack\": 1, \"defense\": 69, \"accuracy\": 1, \"critical\": 1, \"movement\": 1, \"strength\": 6, \"vitality\": 6, \"dexterity\": 6, \"mana_bonus\": 110, \"resistance\": 1, \"health_bonus\": 110, \"intelligence\": 6, \"magic_attack\": 1, \"magic_defense\": 54, \"double_attack_rate\": 1}'),(283,32,46,1,-283,NULL,1,100,NULL,'2026-07-01 04:54:42','2026-07-01 05:30:16',12,'{\"attack\": 96, \"accuracy\": 12, \"critical\": 7, \"strength\": 6, \"dexterity\": 3, \"health_bonus\": 41, \"magic_attack\": 26, \"double_attack_rate\": 3}'),(284,32,38,1,-284,NULL,1,100,NULL,'2026-07-01 04:55:31','2026-07-01 05:26:18',12,'{\"luck\": 28, \"dodge\": 28, \"attack\": 55, \"defense\": 55, \"accuracy\": 28, \"critical\": 28, \"movement\": 14, \"strength\": 28, \"vitality\": 28, \"dexterity\": 28, \"mana_bonus\": 275, \"resistance\": 28, \"health_bonus\": 275, \"intelligence\": 28, \"magic_attack\": 55, \"magic_defense\": 55, \"double_attack_rate\": 28, \"double_attack_resistance\": 28}'),(285,32,7,2,10,NULL,0,100,NULL,'2026-07-01 04:55:57','2026-07-04 01:17:34',0,NULL),(286,32,9,99,11,NULL,0,100,NULL,'2026-07-01 04:56:03','2026-07-01 04:56:03',0,NULL),(287,32,10,92,12,NULL,0,100,NULL,'2026-07-01 04:56:07','2026-07-04 01:16:21',0,NULL),(288,32,11,94,13,NULL,0,100,NULL,'2026-07-01 04:56:33','2026-07-02 18:23:03',0,NULL),(289,32,12,94,14,NULL,0,100,NULL,'2026-07-01 04:56:37','2026-07-02 18:23:05',0,NULL),(290,32,13,94,15,NULL,0,100,NULL,'2026-07-01 04:56:46','2026-07-02 18:23:07',0,NULL),(291,32,14,94,16,NULL,0,100,NULL,'2026-07-01 04:56:49','2026-07-02 18:23:08',0,NULL),(292,32,15,99,17,NULL,0,100,NULL,'2026-07-01 04:56:56','2026-07-01 04:56:56',0,NULL),(294,32,52,817,26,NULL,0,100,NULL,'2026-07-01 04:57:36','2026-07-02 18:22:31',0,NULL),(295,32,53,450,21,NULL,0,100,NULL,'2026-07-01 04:57:40','2026-07-02 18:22:49',0,NULL),(296,32,53,1,-100000038,38,0,100,NULL,'2026-07-01 17:35:03','2026-07-01 17:35:09',0,NULL),(297,32,47,1,0,NULL,0,100,NULL,'2026-07-01 18:35:28','2026-07-02 18:22:49',12,'{\"dodge\": 4, \"attack\": 41, \"accuracy\": 7, \"critical\": 4, \"strength\": 6, \"vitality\": 6, \"mana_bonus\": 41, \"health_bonus\": 41, \"intelligence\": 6, \"magic_attack\": 41, \"double_attack_rate\": 2}'),(298,32,47,1,3,NULL,0,100,NULL,'2026-07-01 18:35:33','2026-07-01 18:51:38',4,'{\"dodge\": 1, \"attack\": 11, \"accuracy\": 2, \"critical\": 1, \"strength\": 2, \"vitality\": 2, \"mana_bonus\": 11, \"health_bonus\": 11, \"intelligence\": 2, \"magic_attack\": 11, \"double_attack_rate\": 1}'),(314,1,10,46,0,NULL,0,100,NULL,'2026-07-03 04:20:19','2026-07-04 04:57:18',0,NULL),(318,1,10,78,10,NULL,0,100,NULL,'2026-07-03 04:25:08','2026-07-03 04:27:06',0,NULL),(319,1,9,97,11,NULL,0,100,NULL,'2026-07-03 04:26:57','2026-07-03 23:31:55',0,NULL),(322,1,7,740,7,NULL,0,100,NULL,'2026-07-03 14:35:46','2026-07-04 01:17:34',0,NULL),(324,1,7,119,8,NULL,0,100,NULL,'2026-07-03 16:50:29','2026-07-03 16:53:14',0,NULL),(326,1,1,1,3,NULL,0,100,NULL,'2026-07-03 16:50:48','2026-07-03 16:50:48',0,NULL),(327,1,5,1,4,NULL,0,100,NULL,'2026-07-03 16:50:52','2026-07-03 16:50:52',0,NULL),(330,23,9,99,0,NULL,0,100,NULL,'2026-07-03 21:32:20','2026-07-03 21:32:20',0,NULL),(331,23,10,99,5,NULL,0,100,NULL,'2026-07-03 21:32:24','2026-07-03 21:32:24',0,NULL),(333,25,7,1,1,NULL,0,100,NULL,'2026-07-03 22:57:17','2026-07-03 22:57:23',0,NULL),(334,25,7,2,5,NULL,0,100,NULL,'2026-07-03 22:57:23','2026-07-03 22:57:23',0,NULL),(335,1,7,38,9,NULL,0,100,NULL,'2026-07-03 23:30:23','2026-07-03 23:30:23',0,NULL),(337,1,13,16,12,NULL,0,100,NULL,'2026-07-03 23:41:47','2026-07-03 23:41:47',0,NULL),(338,1,9,45,13,NULL,0,100,NULL,'2026-07-04 01:05:06','2026-07-04 01:05:31',0,NULL),(341,1,14,17,14,NULL,0,100,NULL,'2026-07-04 01:06:01','2026-07-04 01:06:01',0,NULL),(348,1,1,1,5,NULL,0,100,NULL,'2026-07-04 04:52:46','2026-07-04 04:52:46',0,NULL),(349,25,14,13,7,NULL,0,100,NULL,'2026-07-04 15:04:45','2026-07-04 15:04:48',0,NULL),(350,27,1,1,1,NULL,0,100,NULL,'2026-07-05 01:19:32','2026-07-05 01:19:32',0,NULL);
/*!40000 ALTER TABLE `player_inventory` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_item_buffs`
--

DROP TABLE IF EXISTS `player_item_buffs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_item_buffs` (
  `player_id` bigint unsigned NOT NULL,
  `buff_key` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'Ex: strength_buff, attack_buff',
  `item_template_id` int NOT NULL,
  `item_subtype` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'buff_potion',
  `bonus_value` int NOT NULL DEFAULT '0',
  `duration_ms` int unsigned NOT NULL,
  `started_at_ms` bigint unsigned NOT NULL,
  `expires_at_ms` bigint unsigned NOT NULL,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`,`buff_key`),
  KEY `fk_pib_item_template` (`item_template_id`),
  KEY `idx_pib_expires` (`expires_at_ms`),
  CONSTRAINT `fk_pib_item_template` FOREIGN KEY (`item_template_id`) REFERENCES `item_templates` (`item_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_pib_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_item_buffs`
--

LOCK TABLES `player_item_buffs` WRITE;
/*!40000 ALTER TABLE `player_item_buffs` DISABLE KEYS */;
INSERT INTO `player_item_buffs` VALUES (1,'dexterity_buff',11,'buff_potion',20,300000,1783127050214,1783127350214,'2026-07-04 01:04:10'),(1,'intelligence_buff',12,'buff_potion',20,300000,1783127052993,1783127352993,'2026-07-04 01:04:13'),(1,'luck_buff',14,'buff_potion',20,300000,1783127054818,1783127354818,'2026-07-04 01:04:14'),(1,'strength_buff',10,'buff_potion',20,300000,1783141038371,1783141338371,'2026-07-04 04:57:18'),(1,'vitality_buff',13,'buff_potion',20,300000,1783127051838,1783127351838,'2026-07-04 01:04:11'),(23,'dexterity_buff',11,'buff_potion',20,300000,1783114332479,1783114632479,'2026-07-03 21:32:12'),(25,'dexterity_buff',11,'buff_potion',20,300000,1783119210156,1783119510156,'2026-07-03 22:53:30'),(25,'intelligence_buff',12,'buff_potion',20,300000,1782092694750,1782092994750,'2026-06-22 01:44:54'),(25,'luck_buff',14,'buff_potion',20,300000,1783177488117,1783177788117,'2026-07-04 15:04:48'),(25,'strength_buff',10,'buff_potion',20,300000,1783119207873,1783119507873,'2026-07-03 22:53:27'),(25,'vitality_buff',13,'buff_potion',20,300000,1783119211189,1783119511189,'2026-07-03 22:53:31'),(32,'dexterity_buff',11,'buff_potion',20,300000,1783016583958,1783016883958,'2026-07-02 18:23:03'),(32,'intelligence_buff',12,'buff_potion',20,300000,1783016585991,1783016885991,'2026-07-02 18:23:05'),(32,'luck_buff',14,'buff_potion',20,300000,1783016588788,1783016888788,'2026-07-02 18:23:08'),(32,'strength_buff',10,'buff_potion',20,300000,1783127781784,1783128081784,'2026-07-04 01:16:21'),(32,'vitality_buff',13,'buff_potion',20,300000,1783016587142,1783016887142,'2026-07-02 18:23:07');
/*!40000 ALTER TABLE `player_item_buffs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_levels`
--

DROP TABLE IF EXISTS `player_levels`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_levels` (
  `level_id` int unsigned NOT NULL AUTO_INCREMENT,
  `level_number` int unsigned NOT NULL COMMENT 'Número do nível (1-50)',
  `exp_required` int unsigned NOT NULL COMMENT 'EXP necessário para alcançar este nível (EXP total acumulado)',
  `exp_for_next_level` int unsigned NOT NULL COMMENT 'EXP necessário para passar deste nível para o próximo',
  `stat_points_gained` int unsigned DEFAULT '10' COMMENT 'Pontos de atributos ganhos neste nível',
  `hp_gain` int unsigned DEFAULT '20' COMMENT 'HP máximo ganho neste nível',
  `mp_gain` int unsigned DEFAULT '20' COMMENT 'MP máximo ganho neste nível',
  `phys_atk_gain` int unsigned DEFAULT '5' COMMENT 'Ataque físico ganho neste nível',
  `mag_atk_gain` int unsigned DEFAULT '5' COMMENT 'Ataque mágico ganho neste nível',
  `phys_def_gain` int unsigned DEFAULT '3' COMMENT 'Defesa física ganha neste nível',
  `mag_def_gain` int unsigned DEFAULT '3' COMMENT 'Defesa mágica ganha neste nível',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`level_id`),
  UNIQUE KEY `level_number` (`level_number`),
  UNIQUE KEY `unique_level_number` (`level_number`),
  KEY `idx_level_number` (`level_number`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_levels`
--

LOCK TABLES `player_levels` WRITE;
/*!40000 ALTER TABLE `player_levels` DISABLE KEYS */;
INSERT INTO `player_levels` VALUES (1,1,0,1000,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(2,2,1000,1150,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(3,3,2150,1323,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(4,4,3473,1521,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(5,5,4994,1750,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(6,6,6744,2013,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(7,7,8757,2315,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(8,8,11072,2662,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(9,9,13734,3061,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(10,10,16795,3520,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(11,11,20315,4048,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(12,12,24363,4655,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(13,13,29018,5353,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(14,14,34371,6156,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(15,15,40527,7080,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(16,16,47607,8142,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(17,17,55749,9363,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(18,18,65112,10777,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(19,19,75889,12394,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(20,20,88283,14253,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(21,21,102536,16391,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(22,22,118927,18850,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(23,23,137777,21677,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(24,24,159454,24929,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(25,25,184383,28669,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(26,26,213052,32969,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(27,27,246021,37914,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(28,28,283935,43601,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(29,29,327536,50141,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(30,30,377677,57662,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(31,31,435339,66311,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(32,32,501650,76258,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(33,33,577908,87697,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(34,34,665605,100851,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(35,35,766456,115979,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(36,36,882435,133376,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(37,37,1015811,153382,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(38,38,1169193,176390,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(39,39,1345583,202848,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(40,40,1548431,233276,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(41,41,1781707,268267,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(42,42,2049974,308507,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(43,43,2358481,354793,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(44,44,2713274,408012,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(45,45,3121286,469214,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(46,46,3590500,539596,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(47,47,4130096,620535,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(48,48,4750631,713615,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(49,49,5464246,820658,10,20,20,5,5,3,3,'2025-11-27 20:27:15'),(50,50,6284904,0,10,20,20,5,5,3,3,'2025-11-27 20:27:15');
/*!40000 ALTER TABLE `player_levels` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_quest_objectives`
--

DROP TABLE IF EXISTS `player_quest_objectives`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_quest_objectives` (
  `player_objective_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_quest_id` bigint unsigned NOT NULL,
  `objective_id` int unsigned NOT NULL,
  `current_count` int NOT NULL DEFAULT '0',
  `is_completed` tinyint(1) NOT NULL DEFAULT '0',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_objective_id`),
  UNIQUE KEY `uk_player_quest_objective` (`player_quest_id`,`objective_id`),
  KEY `idx_pqo_objective` (`objective_id`),
  CONSTRAINT `fk_pqo_objective` FOREIGN KEY (`objective_id`) REFERENCES `quest_objectives` (`objective_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_pqo_player_quest` FOREIGN KEY (`player_quest_id`) REFERENCES `player_quests` (`player_quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=75 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_quest_objectives`
--

LOCK TABLES `player_quest_objectives` WRITE;
/*!40000 ALTER TABLE `player_quest_objectives` DISABLE KEYS */;
INSERT INTO `player_quest_objectives` VALUES (5,3,5,1,1,'2026-07-03 20:04:19'),(6,3,6,1,1,'2026-07-03 20:40:51'),(7,4,1,1,1,'2026-07-03 20:07:53'),(8,4,2,3,1,'2026-07-03 21:23:11'),(9,4,3,3,1,'2026-07-03 21:23:11'),(10,5,1,1,1,'2026-07-03 21:30:03'),(11,5,2,3,1,'2026-07-03 21:31:04'),(12,5,3,3,1,'2026-07-03 21:31:04'),(13,6,1,1,1,'2026-07-03 22:54:29'),(14,6,2,3,1,'2026-07-03 22:56:37'),(15,6,3,3,1,'2026-07-03 22:56:37'),(16,7,1,1,1,'2026-07-03 22:54:31'),(17,7,2,3,1,'2026-07-03 22:56:37'),(18,7,3,3,1,'2026-07-03 22:56:37'),(19,8,1,1,1,'2026-07-03 22:54:33'),(20,8,2,3,1,'2026-07-03 22:56:37'),(21,8,3,3,1,'2026-07-03 22:56:37'),(22,9,5,1,1,'2026-07-03 22:57:21'),(23,9,6,1,1,'2026-07-03 22:57:21'),(24,10,1,1,1,'2026-07-03 22:58:48'),(25,10,2,3,1,'2026-07-03 23:00:44'),(26,10,3,3,1,'2026-07-03 23:00:44'),(27,11,1,1,1,'2026-07-04 00:26:59'),(28,11,2,3,1,'2026-07-04 00:56:48'),(29,11,3,3,1,'2026-07-04 00:56:35'),(30,12,1,1,1,'2026-07-04 01:07:47'),(31,12,2,1,0,'2026-07-04 01:08:42'),(32,12,3,0,0,'2026-07-04 01:07:47'),(33,13,5,1,1,'2026-07-04 01:11:31'),(34,13,6,1,1,'2026-07-04 01:11:31'),(35,14,1,1,1,'2026-07-04 03:09:01'),(36,14,2,3,1,'2026-07-04 03:10:23'),(37,14,3,3,1,'2026-07-04 03:10:55'),(38,15,5,1,1,'2026-07-04 03:26:24'),(39,15,6,1,1,'2026-07-04 03:26:25'),(42,17,9,10,1,'2026-07-04 04:33:49'),(43,18,9,10,1,'2026-07-04 04:52:03'),(44,19,1,1,1,'2026-07-04 15:00:39'),(45,19,2,0,0,'2026-07-04 15:00:39'),(46,19,3,0,0,'2026-07-04 15:00:39'),(47,20,9,2,0,'2026-07-04 15:04:03'),(48,21,1,1,1,'2026-07-05 01:13:20'),(49,21,2,3,1,'2026-07-05 01:15:14'),(50,21,3,3,1,'2026-07-05 01:15:57'),(51,22,9,10,1,'2026-07-05 01:18:58'),(52,23,1,1,1,'2026-07-05 02:48:01'),(53,23,2,3,1,'2026-07-05 02:49:37'),(54,23,3,3,1,'2026-07-05 02:50:46'),(55,24,9,2,0,'2026-07-05 02:53:02'),(56,25,5,1,1,'2026-07-05 02:53:56'),(57,25,6,1,1,'2026-07-05 02:53:56'),(58,26,5,1,1,'2026-07-05 18:23:31'),(59,26,6,1,1,'2026-07-05 18:23:31'),(60,27,7,1,1,'2026-07-06 03:45:42'),(61,27,8,1,1,'2026-07-06 03:45:42'),(62,28,5,0,0,'2026-07-05 18:36:17'),(63,28,6,0,0,'2026-07-05 18:36:17'),(64,29,9,0,0,'2026-07-05 18:36:32'),(65,30,1,1,1,'2026-07-05 22:26:15'),(66,30,2,0,0,'2026-07-05 22:26:15'),(67,30,3,0,0,'2026-07-05 22:26:15'),(68,31,5,0,0,'2026-07-05 22:30:12'),(69,31,6,0,0,'2026-07-05 22:30:12'),(70,32,1,1,1,'2026-07-06 18:06:52'),(71,32,2,1,0,'2026-07-06 18:08:14'),(72,32,3,0,0,'2026-07-06 18:06:52'),(73,33,5,0,0,'2026-07-06 18:06:57'),(74,33,6,0,0,'2026-07-06 18:06:57');
/*!40000 ALTER TABLE `player_quest_objectives` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_quests`
--

DROP TABLE IF EXISTS `player_quests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_quests` (
  `player_quest_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_id` int unsigned NOT NULL,
  `quest_id` int unsigned NOT NULL,
  `status` enum('active','ready','completed','failed') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'active',
  `accepted_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `completed_at` timestamp NULL DEFAULT NULL,
  `chosen_rewards_json` json DEFAULT NULL,
  PRIMARY KEY (`player_quest_id`),
  KEY `idx_player_quests_player` (`player_id`),
  KEY `idx_player_quests_quest` (`quest_id`),
  KEY `idx_player_quests_status` (`player_id`,`status`),
  CONSTRAINT `fk_player_quests_quest` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=34 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_quests`
--

LOCK TABLES `player_quests` WRITE;
/*!40000 ALTER TABLE `player_quests` DISABLE KEYS */;
INSERT INTO `player_quests` VALUES (3,1,2,'completed','2026-07-03 20:04:19','2026-07-03 20:45:34',NULL),(4,1,1,'completed','2026-07-03 20:07:53','2026-07-03 21:23:30',NULL),(5,23,1,'completed','2026-07-03 21:30:03','2026-07-03 21:31:19',NULL),(6,25,1,'ready','2026-07-03 22:54:28',NULL,NULL),(7,25,1,'ready','2026-07-03 22:54:31',NULL,NULL),(8,25,1,'completed','2026-07-03 22:54:33','2026-07-03 22:56:50',NULL),(9,25,2,'completed','2026-07-03 22:57:06','2026-07-03 22:57:23',NULL),(10,45,1,'completed','2026-07-03 22:58:47','2026-07-03 23:00:58',NULL),(11,62,1,'ready','2026-07-04 00:26:59',NULL,NULL),(12,63,1,'active','2026-07-04 01:07:47',NULL,NULL),(13,63,2,'completed','2026-07-04 01:09:57','2026-07-04 01:11:35',NULL),(14,59,1,'completed','2026-07-04 03:09:01','2026-07-04 03:25:58',NULL),(15,59,2,'completed','2026-07-04 03:25:41','2026-07-04 03:26:31',NULL),(17,59,4,'completed','2026-07-04 04:26:14','2026-07-04 04:34:21',NULL),(18,1,4,'completed','2026-07-04 04:50:09','2026-07-04 04:52:46',NULL),(19,64,1,'active','2026-07-04 15:00:39',NULL,NULL),(20,25,4,'active','2026-07-04 15:02:49',NULL,NULL),(21,27,1,'completed','2026-07-05 01:13:20','2026-07-05 01:16:22',NULL),(22,27,4,'completed','2026-07-05 01:16:37','2026-07-05 01:19:32',NULL),(23,61,1,'completed','2026-07-05 02:48:01','2026-07-05 02:50:57',NULL),(24,61,4,'active','2026-07-05 02:51:29',NULL,NULL),(25,61,2,'completed','2026-07-05 02:53:56','2026-07-05 02:53:58',NULL),(26,27,2,'ready','2026-07-05 18:23:31',NULL,NULL),(27,1,3,'ready','2026-07-05 18:26:16',NULL,NULL),(28,23,2,'active','2026-07-05 18:36:17',NULL,NULL),(29,23,4,'active','2026-07-05 18:36:32',NULL,NULL),(30,65,1,'active','2026-07-05 22:26:15',NULL,NULL),(31,65,2,'active','2026-07-05 22:30:12',NULL,NULL),(32,66,1,'active','2026-07-06 18:06:52',NULL,NULL),(33,66,2,'active','2026-07-06 18:06:57',NULL,NULL);
/*!40000 ALTER TABLE `player_quests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_reports`
--

DROP TABLE IF EXISTS `player_reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_reports` (
  `report_id` int unsigned NOT NULL AUTO_INCREMENT,
  `reporter_id` bigint unsigned NOT NULL COMMENT 'Jogador que denunciou',
  `reported_id` bigint unsigned NOT NULL COMMENT 'Jogador denunciado',
  `reason` text COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'Motivo da denúncia',
  `status` enum('pending','reviewed','resolved','dismissed') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `reviewed_at` timestamp NULL DEFAULT NULL,
  `reviewed_by` bigint unsigned DEFAULT NULL COMMENT 'Admin que revisou',
  PRIMARY KEY (`report_id`),
  KEY `idx_reporter` (`reporter_id`),
  KEY `idx_reported` (`reported_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `player_reports_ibfk_1` FOREIGN KEY (`reporter_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `player_reports_ibfk_2` FOREIGN KEY (`reported_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_reports`
--

LOCK TABLES `player_reports` WRITE;
/*!40000 ALTER TABLE `player_reports` DISABLE KEYS */;
INSERT INTO `player_reports` VALUES (4,1,32,'aaaaaaaaaaaaaaaaaaa','pending','2026-02-24 15:12:49',NULL,NULL),(5,23,1,'testando report','pending','2026-03-02 18:47:50',NULL,NULL),(6,23,25,'é um ladrão e vacilão','pending','2026-04-08 23:09:51',NULL,NULL);
/*!40000 ALTER TABLE `player_reports` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_resistances`
--

DROP TABLE IF EXISTS `player_resistances`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_resistances` (
  `player_id` bigint unsigned NOT NULL,
  `physical_res` smallint NOT NULL DEFAULT '0',
  `shadow_res` smallint NOT NULL DEFAULT '0',
  `fire_res` smallint NOT NULL DEFAULT '0',
  `holy_res` smallint NOT NULL DEFAULT '0',
  `poison_res` smallint NOT NULL DEFAULT '0',
  `ice_res` smallint NOT NULL DEFAULT '0',
  `lightning_res` smallint NOT NULL DEFAULT '0',
  `arcane_res` smallint NOT NULL DEFAULT '0',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`),
  CONSTRAINT `fk_resistances_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_resistances`
--

LOCK TABLES `player_resistances` WRITE;
/*!40000 ALTER TABLE `player_resistances` DISABLE KEYS */;
/*!40000 ALTER TABLE `player_resistances` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_sessions`
--

DROP TABLE IF EXISTS `player_sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_sessions` (
  `player_id` int unsigned NOT NULL COMMENT 'ID do jogador (players.id)',
  `last_seen` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Última vez que reportou online',
  PRIMARY KEY (`player_id`),
  KEY `idx_last_seen` (`last_seen`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_sessions`
--

LOCK TABLES `player_sessions` WRITE;
/*!40000 ALTER TABLE `player_sessions` DISABLE KEYS */;
/*!40000 ALTER TABLE `player_sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_skill_points`
--

DROP TABLE IF EXISTS `player_skill_points`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_skill_points` (
  `player_id` bigint unsigned NOT NULL,
  `total_points_earned` int unsigned NOT NULL DEFAULT '0',
  `points_spent` int unsigned NOT NULL DEFAULT '0',
  `points_available` int unsigned NOT NULL DEFAULT '0',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`),
  CONSTRAINT `fk_skillpoints_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_skill_points`
--

LOCK TABLES `player_skill_points` WRITE;
/*!40000 ALTER TABLE `player_skill_points` DISABLE KEYS */;
INSERT INTO `player_skill_points` VALUES (1,150,75,75,'2026-02-24 19:49:22'),(4,3,0,3,'2026-02-23 15:24:36'),(23,33,4,29,'2026-07-05 18:37:26'),(25,50,29,21,'2026-05-17 00:16:22'),(27,100,24,76,'2026-06-22 02:44:10'),(31,3,1,2,'2026-06-19 19:38:52'),(32,150,22,128,'2026-07-02 17:51:13'),(33,3,2,1,'2026-04-09 18:04:40'),(34,3,1,2,'2026-02-25 00:37:54'),(44,3,0,3,'2026-04-19 07:30:24'),(45,18,1,17,'2026-07-03 22:58:16'),(46,3,1,2,'2026-05-06 16:12:11'),(49,3,0,3,'2026-05-10 13:47:52'),(62,78,7,71,'2026-07-04 00:52:14'),(64,12,1,11,'2026-07-04 15:01:47'),(65,6,0,6,'2026-07-05 22:30:33'),(66,21,1,20,'2026-07-06 18:06:36');
/*!40000 ALTER TABLE `player_skill_points` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_skillbar`
--

DROP TABLE IF EXISTS `player_skillbar`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_skillbar` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `slot_index` tinyint unsigned NOT NULL,
  `skill_id` int unsigned DEFAULT NULL,
  `item_template_id` int DEFAULT NULL COMMENT 'Consumível na barra (mutuamente exclusivo com skill_id)',
  `keybind` varchar(20) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_player_slot` (`player_id`,`slot_index`),
  KEY `idx_player_skillbar` (`player_id`),
  KEY `fk_skillbar_skill` (`skill_id`),
  KEY `fk_skillbar_item_template` (`item_template_id`),
  CONSTRAINT `fk_skillbar_item_template` FOREIGN KEY (`item_template_id`) REFERENCES `item_templates` (`item_id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `fk_skillbar_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_skillbar_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `chk_slot_index` CHECK (((`slot_index` >= 0) and (`slot_index` < 20)))
) ENGINE=InnoDB AUTO_INCREMENT=1217 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_skillbar`
--

LOCK TABLES `player_skillbar` WRITE;
/*!40000 ALTER TABLE `player_skillbar` DISABLE KEYS */;
INSERT INTO `player_skillbar` VALUES (1,1,0,91,NULL,'One','2026-06-21 22:12:40'),(2,1,1,1,NULL,'Two','2026-06-21 22:12:42'),(3,1,2,13,NULL,'Three','2026-06-21 22:12:43'),(4,1,3,7,NULL,'Four','2026-06-21 22:12:45'),(5,1,4,3,NULL,'Five','2026-06-21 22:12:47'),(6,1,5,10,NULL,'Six','2026-06-27 18:37:09'),(7,1,6,9,NULL,'Seven','2026-06-27 18:37:11'),(8,1,7,13,NULL,NULL,'2026-05-21 23:23:06'),(9,1,8,NULL,7,'F','2026-05-31 02:50:30'),(10,1,9,1,NULL,NULL,'2026-06-22 15:27:25'),(11,1,10,NULL,10,NULL,'2026-05-21 23:23:18'),(12,1,11,15,NULL,'Q','2026-07-01 05:04:12'),(13,1,12,5,NULL,'Nine','2026-07-02 20:05:07'),(14,1,13,11,NULL,'R','2026-07-01 05:05:27'),(15,1,14,NULL,9,NULL,'2026-06-21 20:29:54'),(16,1,15,NULL,13,NULL,'2026-05-21 23:22:57'),(17,1,16,10,NULL,NULL,'2026-05-20 18:06:51'),(18,1,17,NULL,12,'Up','2026-05-21 23:10:57'),(19,1,18,NULL,14,'Down','2026-05-21 23:10:59'),(20,1,19,NULL,NULL,NULL,'2026-06-20 03:07:27'),(41,23,0,95,NULL,'One','2026-06-23 02:11:18'),(42,23,1,76,NULL,'Two','2026-07-03 21:30:16'),(43,23,2,76,NULL,NULL,'2026-05-17 00:10:37'),(44,23,3,76,NULL,NULL,'2026-06-23 02:11:13'),(45,23,4,77,NULL,NULL,'2026-06-24 18:21:44'),(46,23,5,NULL,NULL,NULL,'2026-02-25 12:29:41'),(47,23,6,NULL,NULL,NULL,'2026-02-25 12:29:41'),(48,23,7,NULL,NULL,NULL,'2026-02-25 12:29:41'),(49,23,8,NULL,NULL,NULL,'2026-04-08 20:54:18'),(50,23,9,NULL,NULL,NULL,'2026-04-08 20:55:11'),(51,23,10,NULL,NULL,NULL,'2026-04-08 20:55:00'),(52,23,11,NULL,NULL,NULL,'2026-04-08 20:55:03'),(53,23,12,NULL,NULL,NULL,'2026-04-08 20:54:20'),(54,23,13,NULL,NULL,NULL,'2026-02-25 12:29:41'),(55,23,14,NULL,NULL,NULL,'2026-02-25 12:29:41'),(56,23,15,NULL,NULL,NULL,'2026-02-25 12:29:41'),(57,23,16,NULL,NULL,NULL,'2026-02-25 12:29:41'),(58,23,17,NULL,NULL,NULL,'2026-02-25 12:29:41'),(59,23,18,NULL,NULL,NULL,'2026-02-25 12:29:41'),(60,23,19,NULL,NULL,NULL,'2026-04-08 20:55:12'),(61,25,0,93,NULL,'One','2026-07-04 15:03:18'),(62,25,1,16,NULL,'Two','2026-07-03 22:55:39'),(63,25,2,17,NULL,'Three','2026-07-03 22:55:34'),(64,25,3,18,NULL,'Four','2026-07-03 22:55:37'),(65,25,4,20,NULL,'Five','2026-07-03 22:55:37'),(66,25,5,22,NULL,'Six','2026-07-03 22:55:48'),(67,25,6,24,NULL,NULL,'2026-04-08 12:48:46'),(68,25,7,NULL,7,NULL,'2026-07-03 22:55:28'),(69,25,8,NULL,12,NULL,'2026-06-01 01:35:07'),(70,25,9,NULL,NULL,NULL,'2026-02-26 14:51:40'),(71,25,10,16,NULL,NULL,'2026-04-08 03:08:56'),(72,25,11,22,NULL,NULL,'2026-06-21 02:31:42'),(73,25,12,20,NULL,NULL,'2026-04-08 12:48:21'),(74,25,13,22,NULL,NULL,'2026-06-21 02:34:22'),(75,25,14,17,NULL,NULL,'2026-02-27 00:33:28'),(76,25,15,NULL,NULL,NULL,'2026-04-08 03:09:03'),(77,25,16,NULL,NULL,NULL,'2026-02-26 14:51:40'),(78,25,17,16,NULL,NULL,'2026-04-08 12:49:08'),(79,25,18,NULL,NULL,NULL,'2026-02-26 14:51:40'),(80,25,19,NULL,NULL,NULL,'2026-02-26 14:51:40'),(81,33,0,61,NULL,NULL,'2026-04-09 18:04:44'),(82,33,1,NULL,NULL,NULL,'2026-02-26 14:53:56'),(83,33,2,NULL,NULL,NULL,'2026-02-26 14:53:56'),(84,33,3,NULL,NULL,NULL,'2026-02-26 14:53:56'),(85,33,4,NULL,NULL,NULL,'2026-02-26 14:53:56'),(86,33,5,NULL,NULL,NULL,'2026-02-26 14:53:56'),(87,33,6,NULL,NULL,NULL,'2026-02-26 14:53:56'),(88,33,7,NULL,NULL,NULL,'2026-02-26 14:53:56'),(89,33,8,NULL,NULL,NULL,'2026-02-26 14:53:56'),(90,33,9,NULL,NULL,NULL,'2026-02-26 14:53:56'),(91,33,10,NULL,NULL,NULL,'2026-02-26 14:53:56'),(92,33,11,NULL,NULL,NULL,'2026-02-26 14:53:56'),(93,33,12,NULL,NULL,NULL,'2026-02-26 14:53:56'),(94,33,13,NULL,NULL,NULL,'2026-02-26 14:53:56'),(95,33,14,NULL,NULL,NULL,'2026-02-26 14:53:56'),(96,33,15,NULL,NULL,NULL,'2026-02-26 14:53:56'),(97,33,16,NULL,NULL,NULL,'2026-02-26 14:53:56'),(98,33,17,NULL,NULL,NULL,'2026-02-26 14:53:56'),(99,33,18,NULL,NULL,NULL,'2026-02-26 14:53:56'),(100,33,19,NULL,NULL,NULL,'2026-02-26 14:53:56'),(185,27,0,92,NULL,'One','2026-06-22 02:46:29'),(186,27,1,52,NULL,'Two','2026-07-05 01:14:36'),(187,27,2,46,NULL,'Three','2026-07-05 01:14:38'),(188,27,3,49,NULL,'Four','2026-07-05 01:14:40'),(189,27,4,50,NULL,NULL,'2026-06-22 02:42:52'),(190,27,5,51,NULL,NULL,'2026-06-22 02:42:59'),(191,27,6,60,NULL,NULL,'2026-07-05 01:14:10'),(192,27,7,53,NULL,NULL,'2026-06-22 02:43:13'),(193,27,8,54,NULL,NULL,'2026-06-22 02:43:20'),(194,27,9,NULL,NULL,NULL,'2026-02-27 16:45:09'),(195,27,10,55,NULL,NULL,'2026-06-22 02:43:29'),(196,27,11,57,NULL,NULL,'2026-06-22 02:43:46'),(197,27,12,58,NULL,NULL,'2026-06-22 02:43:53'),(198,27,13,47,NULL,NULL,'2026-07-05 01:13:43'),(199,27,14,48,NULL,NULL,'2026-07-05 01:13:34'),(200,27,15,NULL,NULL,NULL,'2026-02-27 16:45:09'),(201,27,16,NULL,NULL,NULL,'2026-02-27 16:45:09'),(202,27,17,NULL,NULL,NULL,'2026-02-27 16:45:09'),(203,27,18,NULL,NULL,NULL,'2026-02-27 16:45:09'),(204,27,19,NULL,NULL,NULL,'2026-02-27 16:45:09'),(462,44,0,NULL,NULL,NULL,'2026-04-19 07:30:34'),(463,44,1,NULL,NULL,NULL,'2026-04-19 07:30:34'),(464,44,2,NULL,NULL,NULL,'2026-04-19 07:30:34'),(465,44,3,NULL,NULL,NULL,'2026-04-19 07:30:34'),(466,44,4,NULL,NULL,NULL,'2026-04-19 07:30:34'),(467,44,5,NULL,NULL,NULL,'2026-04-19 07:30:34'),(468,44,6,NULL,NULL,NULL,'2026-04-19 07:30:34'),(469,44,7,NULL,NULL,NULL,'2026-04-19 07:30:34'),(470,44,8,NULL,NULL,NULL,'2026-04-19 07:30:34'),(471,44,9,NULL,NULL,NULL,'2026-04-19 07:30:34'),(472,44,10,NULL,NULL,NULL,'2026-04-19 07:30:34'),(473,44,11,NULL,NULL,NULL,'2026-04-19 07:30:34'),(474,44,12,NULL,NULL,NULL,'2026-04-19 07:30:34'),(475,44,13,NULL,NULL,NULL,'2026-04-19 07:30:34'),(476,44,14,NULL,NULL,NULL,'2026-04-19 07:30:34'),(477,44,15,NULL,NULL,NULL,'2026-04-19 07:30:34'),(478,44,16,NULL,NULL,NULL,'2026-04-19 07:30:34'),(479,44,17,NULL,NULL,NULL,'2026-04-19 07:30:34'),(480,44,18,NULL,NULL,NULL,'2026-04-19 07:30:34'),(481,44,19,NULL,NULL,NULL,'2026-04-19 07:30:34'),(482,45,0,95,NULL,'One','2026-07-03 22:58:23'),(483,45,1,76,NULL,'Two','2026-07-03 22:58:24'),(484,45,2,NULL,NULL,NULL,'2026-04-27 19:46:18'),(485,45,3,NULL,NULL,NULL,'2026-04-27 19:46:18'),(486,45,4,NULL,NULL,NULL,'2026-04-27 19:46:18'),(487,45,5,NULL,NULL,NULL,'2026-04-27 19:46:18'),(488,45,6,NULL,NULL,NULL,'2026-04-27 19:46:18'),(489,45,7,NULL,NULL,NULL,'2026-04-27 19:46:18'),(490,45,8,NULL,NULL,NULL,'2026-04-27 19:46:18'),(491,45,9,NULL,NULL,NULL,'2026-04-27 19:46:18'),(492,45,10,NULL,NULL,NULL,'2026-04-27 19:46:18'),(493,45,11,NULL,NULL,NULL,'2026-04-27 19:46:18'),(494,45,12,NULL,NULL,NULL,'2026-04-27 19:46:18'),(495,45,13,NULL,NULL,NULL,'2026-04-27 19:46:18'),(496,45,14,NULL,NULL,NULL,'2026-04-27 19:46:18'),(497,45,15,NULL,NULL,NULL,'2026-04-27 19:46:18'),(498,45,16,NULL,NULL,NULL,'2026-04-27 19:46:18'),(499,45,17,NULL,NULL,NULL,'2026-04-27 19:46:18'),(500,45,18,NULL,NULL,NULL,'2026-04-27 19:46:18'),(501,45,19,NULL,NULL,NULL,'2026-04-27 19:46:18'),(502,2,0,NULL,NULL,NULL,'2026-04-27 20:05:18'),(503,2,1,NULL,NULL,NULL,'2026-04-27 20:05:18'),(504,2,2,NULL,NULL,NULL,'2026-04-27 20:05:18'),(505,2,3,NULL,NULL,NULL,'2026-04-27 20:05:18'),(506,2,4,NULL,NULL,NULL,'2026-04-27 20:05:18'),(507,2,5,NULL,NULL,NULL,'2026-04-27 20:05:18'),(508,2,6,NULL,NULL,NULL,'2026-04-27 20:05:18'),(509,2,7,NULL,NULL,NULL,'2026-04-27 20:05:19'),(510,2,8,NULL,NULL,NULL,'2026-04-27 20:05:19'),(511,2,9,NULL,NULL,NULL,'2026-04-27 20:05:19'),(512,2,10,NULL,NULL,NULL,'2026-04-27 20:05:19'),(513,2,11,NULL,NULL,NULL,'2026-04-27 20:05:19'),(514,2,12,NULL,NULL,NULL,'2026-04-27 20:05:19'),(515,2,13,NULL,NULL,NULL,'2026-04-27 20:05:19'),(516,2,14,NULL,NULL,NULL,'2026-04-27 20:05:19'),(517,2,15,NULL,NULL,NULL,'2026-04-27 20:05:19'),(518,2,16,NULL,NULL,NULL,'2026-04-27 20:05:19'),(519,2,17,NULL,NULL,NULL,'2026-04-27 20:05:19'),(520,2,18,NULL,NULL,NULL,'2026-04-27 20:05:19'),(521,2,19,NULL,NULL,NULL,'2026-04-27 20:05:19'),(522,46,0,16,NULL,NULL,'2026-05-06 16:12:14'),(523,46,1,NULL,NULL,NULL,'2026-05-06 16:08:37'),(524,46,2,NULL,NULL,NULL,'2026-05-06 16:08:37'),(525,46,3,NULL,NULL,NULL,'2026-05-06 16:08:37'),(526,46,4,NULL,NULL,NULL,'2026-05-06 16:08:37'),(527,46,5,NULL,NULL,NULL,'2026-05-06 16:08:37'),(528,46,6,NULL,NULL,NULL,'2026-05-06 16:08:37'),(529,46,7,NULL,NULL,NULL,'2026-05-06 16:08:37'),(530,46,8,NULL,NULL,NULL,'2026-05-06 16:08:37'),(531,46,9,NULL,NULL,NULL,'2026-05-06 16:08:37'),(532,46,10,NULL,NULL,NULL,'2026-05-06 16:08:37'),(533,46,11,NULL,NULL,NULL,'2026-05-06 16:08:37'),(534,46,12,NULL,NULL,NULL,'2026-05-06 16:08:37'),(535,46,13,NULL,NULL,NULL,'2026-05-06 16:08:37'),(536,46,14,NULL,NULL,NULL,'2026-05-06 16:08:37'),(537,46,15,NULL,NULL,NULL,'2026-05-06 16:08:37'),(538,46,16,NULL,NULL,NULL,'2026-05-06 16:08:37'),(539,46,17,NULL,NULL,NULL,'2026-05-06 16:08:37'),(540,46,18,NULL,NULL,NULL,'2026-05-06 16:08:37'),(541,46,19,NULL,NULL,NULL,'2026-05-06 16:08:37'),(547,49,0,NULL,NULL,NULL,'2026-05-10 13:48:07'),(548,49,1,NULL,NULL,NULL,'2026-05-10 13:48:07'),(549,49,2,NULL,NULL,NULL,'2026-05-10 13:48:07'),(550,49,3,NULL,NULL,NULL,'2026-05-10 13:48:07'),(551,49,4,NULL,NULL,NULL,'2026-05-10 13:48:07'),(552,49,5,NULL,NULL,NULL,'2026-05-10 13:48:07'),(553,49,6,NULL,NULL,NULL,'2026-05-10 13:48:07'),(554,49,7,NULL,NULL,NULL,'2026-05-10 13:48:07'),(555,49,8,NULL,NULL,NULL,'2026-05-10 13:48:07'),(556,49,9,NULL,NULL,NULL,'2026-05-10 13:48:07'),(557,49,10,NULL,NULL,NULL,'2026-05-10 13:48:07'),(558,49,11,NULL,NULL,NULL,'2026-05-10 13:48:07'),(559,49,12,NULL,NULL,NULL,'2026-05-10 13:48:07'),(560,49,13,NULL,NULL,NULL,'2026-05-10 13:48:07'),(561,49,14,NULL,NULL,NULL,'2026-05-10 13:48:07'),(562,49,15,NULL,NULL,NULL,'2026-05-10 13:48:07'),(563,49,16,NULL,NULL,NULL,'2026-05-10 13:48:07'),(564,49,17,NULL,NULL,NULL,'2026-05-10 13:48:07'),(565,49,18,NULL,NULL,NULL,'2026-05-10 13:48:07'),(566,49,19,NULL,NULL,NULL,'2026-05-10 13:48:07'),(591,4,0,NULL,NULL,NULL,'2026-05-16 00:05:24'),(592,4,1,NULL,NULL,NULL,'2026-05-16 00:05:24'),(593,4,2,NULL,NULL,NULL,'2026-05-16 00:05:24'),(594,4,3,NULL,NULL,NULL,'2026-05-16 00:05:24'),(595,4,4,NULL,NULL,NULL,'2026-05-16 00:05:24'),(596,4,5,NULL,NULL,NULL,'2026-05-16 00:05:24'),(597,4,6,NULL,NULL,NULL,'2026-05-16 00:05:24'),(598,4,7,NULL,NULL,NULL,'2026-05-16 00:05:24'),(599,4,8,NULL,NULL,NULL,'2026-05-16 00:05:24'),(600,4,9,NULL,NULL,NULL,'2026-05-16 00:05:24'),(601,4,10,NULL,NULL,NULL,'2026-05-16 00:05:24'),(602,4,11,NULL,NULL,NULL,'2026-05-16 00:05:24'),(603,4,12,NULL,NULL,NULL,'2026-05-16 00:05:24'),(604,4,13,NULL,NULL,NULL,'2026-05-16 00:05:24'),(605,4,14,NULL,NULL,NULL,'2026-05-16 00:05:24'),(606,4,15,NULL,NULL,NULL,'2026-05-16 00:05:24'),(607,4,16,NULL,NULL,NULL,'2026-05-16 00:05:24'),(608,4,17,NULL,NULL,NULL,'2026-05-16 00:05:24'),(609,4,18,NULL,NULL,NULL,'2026-05-16 00:05:24'),(610,4,19,NULL,NULL,NULL,'2026-05-16 00:05:24'),(611,32,0,96,NULL,'One','2026-07-02 17:42:02'),(612,32,1,31,NULL,'Two','2026-06-27 13:53:09'),(613,32,2,38,NULL,'Three','2026-07-01 05:31:56'),(614,32,3,33,NULL,'Four','2026-06-27 13:53:13'),(615,32,4,34,NULL,'Five','2026-06-27 13:53:14'),(616,32,5,37,NULL,'Six','2026-06-27 13:53:15'),(617,32,6,42,NULL,'Seven','2026-07-01 04:59:28'),(618,32,7,39,NULL,'G','2026-07-01 05:05:14'),(619,32,8,NULL,NULL,NULL,'2026-07-01 04:59:54'),(620,32,9,NULL,7,'F','2026-07-01 05:00:04'),(621,32,10,41,NULL,'LeftShift+Q','2026-06-27 13:53:29'),(622,32,11,40,NULL,'R','2026-07-01 05:05:19'),(623,32,12,43,NULL,'E','2026-07-01 05:04:30'),(624,32,13,45,NULL,'Q','2026-07-01 05:04:10'),(625,32,14,32,NULL,'T','2026-07-01 05:05:22'),(626,32,15,NULL,13,NULL,'2026-07-01 04:58:35'),(627,32,16,NULL,14,NULL,'2026-07-01 04:58:36'),(628,32,17,NULL,10,'Eight','2026-07-02 17:43:06'),(629,32,18,NULL,11,NULL,'2026-07-01 04:58:39'),(630,32,19,NULL,12,NULL,'2026-07-01 04:58:41'),(795,31,0,NULL,NULL,NULL,'2026-06-19 19:38:38'),(796,31,1,NULL,NULL,NULL,'2026-06-19 19:38:38'),(797,31,2,NULL,NULL,NULL,'2026-06-19 19:38:38'),(798,31,3,NULL,NULL,NULL,'2026-06-19 19:38:38'),(799,31,4,NULL,NULL,NULL,'2026-06-19 19:38:38'),(800,31,5,NULL,NULL,NULL,'2026-06-19 19:38:38'),(801,31,6,NULL,NULL,NULL,'2026-06-19 19:38:38'),(802,31,7,NULL,NULL,NULL,'2026-06-19 19:38:38'),(803,31,8,NULL,NULL,NULL,'2026-06-19 19:38:38'),(804,31,9,NULL,NULL,NULL,'2026-06-19 19:38:38'),(805,31,10,NULL,NULL,NULL,'2026-06-19 19:38:38'),(806,31,11,61,NULL,NULL,'2026-06-19 19:38:58'),(807,31,12,NULL,NULL,NULL,'2026-06-19 19:38:38'),(808,31,13,NULL,NULL,NULL,'2026-06-19 19:38:38'),(809,31,14,NULL,NULL,NULL,'2026-06-19 19:38:38'),(810,31,15,NULL,NULL,NULL,'2026-06-19 19:38:38'),(811,31,16,NULL,NULL,NULL,'2026-06-19 19:38:38'),(812,31,17,NULL,NULL,NULL,'2026-06-19 19:38:38'),(813,31,18,NULL,NULL,NULL,'2026-06-19 19:38:38'),(814,31,19,NULL,NULL,NULL,'2026-06-19 19:38:38'),(1058,62,0,94,NULL,'One','2026-07-04 00:27:43'),(1059,62,1,61,NULL,'Two','2026-07-04 00:39:18'),(1060,62,2,62,NULL,'Three','2026-07-04 00:39:20'),(1061,62,3,63,NULL,'Four','2026-07-04 00:39:22'),(1062,62,4,64,NULL,'Five','2026-07-04 00:39:23'),(1063,62,5,65,NULL,'Six','2026-07-04 00:39:24'),(1064,62,6,66,NULL,'Seven','2026-07-04 00:39:27'),(1065,62,7,68,NULL,NULL,'2026-07-04 00:52:19'),(1066,62,8,NULL,NULL,NULL,'2026-07-04 00:26:01'),(1067,62,9,NULL,NULL,NULL,'2026-07-04 00:26:02'),(1068,62,10,NULL,NULL,NULL,'2026-07-04 00:26:02'),(1069,62,11,NULL,NULL,NULL,'2026-07-04 00:26:02'),(1070,62,12,NULL,NULL,NULL,'2026-07-04 00:26:03'),(1071,62,13,NULL,NULL,NULL,'2026-07-04 00:26:04'),(1072,62,14,NULL,NULL,NULL,'2026-07-04 00:26:04'),(1073,62,15,NULL,NULL,NULL,'2026-07-04 00:26:04'),(1074,62,16,NULL,NULL,NULL,'2026-07-04 00:26:04'),(1075,62,17,NULL,NULL,NULL,'2026-07-04 00:26:05'),(1076,62,18,NULL,NULL,NULL,'2026-07-04 00:26:05'),(1077,62,19,NULL,NULL,NULL,'2026-07-04 00:26:06'),(1121,64,0,92,NULL,'One','2026-07-04 15:01:04'),(1122,64,1,46,NULL,'Two','2026-07-04 15:01:06'),(1123,64,2,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1124,64,3,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1125,64,4,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1126,64,5,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1127,64,6,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1128,64,7,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1129,64,8,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1130,64,9,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1131,64,10,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1132,64,11,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1133,64,12,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1134,64,13,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1135,64,14,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1136,64,15,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1137,64,16,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1138,64,17,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1139,64,18,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1140,64,19,NULL,NULL,NULL,'2026-07-04 14:59:59'),(1171,65,0,94,NULL,'One','2026-07-05 22:29:41'),(1172,65,1,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1173,65,2,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1174,65,3,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1175,65,4,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1176,65,5,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1177,65,6,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1178,65,7,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1179,65,8,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1180,65,9,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1181,65,10,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1182,65,11,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1183,65,12,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1184,65,13,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1185,65,14,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1186,65,15,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1187,65,16,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1188,65,17,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1189,65,18,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1190,65,19,NULL,NULL,NULL,'2026-07-05 22:25:57'),(1193,66,0,92,NULL,'One','2026-07-06 18:06:34'),(1194,66,1,46,NULL,'Two','2026-07-06 18:06:37'),(1195,66,2,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1196,66,3,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1197,66,4,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1198,66,5,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1199,66,6,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1200,66,7,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1201,66,8,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1202,66,9,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1203,66,10,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1204,66,11,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1205,66,12,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1206,66,13,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1207,66,14,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1208,66,15,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1209,66,16,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1210,66,17,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1211,66,18,NULL,NULL,NULL,'2026-07-06 18:05:31'),(1212,66,19,NULL,NULL,NULL,'2026-07-06 18:05:31');
/*!40000 ALTER TABLE `player_skillbar` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_skills`
--

DROP TABLE IF EXISTS `player_skills`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_skills` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `skill_id` int unsigned NOT NULL,
  `current_rank` tinyint unsigned NOT NULL DEFAULT '1',
  `learned_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_used_at` timestamp NULL DEFAULT NULL,
  `total_uses` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_player_skill` (`player_id`,`skill_id`),
  KEY `idx_player_skills` (`player_id`),
  KEY `fk_pskills_skill` (`skill_id`),
  CONSTRAINT `fk_pskills_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_pskills_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=110 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_skills`
--

LOCK TABLES `player_skills` WRITE;
/*!40000 ALTER TABLE `player_skills` DISABLE KEYS */;
INSERT INTO `player_skills` VALUES (1,27,46,3,'2026-02-24 03:15:33',NULL,0),(3,1,1,5,'2026-02-24 03:25:27','2026-05-23 01:04:53',26),(4,32,31,3,'2026-02-24 15:13:13','2026-05-21 20:04:39',1),(6,23,76,3,'2026-02-24 19:23:12',NULL,0),(7,33,61,2,'2026-02-24 19:30:20',NULL,0),(8,1,2,5,'2026-02-24 19:43:33',NULL,0),(9,1,3,5,'2026-02-24 19:45:56','2026-05-21 21:34:59',1),(10,1,4,5,'2026-02-24 19:46:15',NULL,0),(11,1,5,5,'2026-02-24 19:46:24','2026-05-31 19:47:39',2),(12,1,6,5,'2026-02-24 19:46:40',NULL,0),(13,1,7,5,'2026-02-24 19:47:54','2026-06-19 13:44:39',346),(14,1,8,5,'2026-02-24 19:48:05',NULL,0),(15,1,9,5,'2026-02-24 19:48:15','2026-05-20 18:40:59',1),(16,1,10,5,'2026-02-24 19:48:28','2026-05-21 23:09:42',1),(17,1,11,5,'2026-02-24 19:48:37',NULL,0),(18,1,12,5,'2026-02-24 19:48:44',NULL,0),(19,1,13,5,'2026-02-24 19:48:55','2026-05-22 00:30:00',3),(20,1,14,5,'2026-02-24 19:49:06',NULL,0),(21,1,15,5,'2026-02-24 19:49:15','2026-05-20 22:26:58',1),(22,25,16,5,'2026-02-24 20:45:28',NULL,0),(23,25,17,5,'2026-02-24 20:49:12',NULL,0),(24,25,19,3,'2026-02-24 20:50:11',NULL,0),(25,25,18,5,'2026-02-24 20:56:55',NULL,0),(26,25,21,2,'2026-02-24 20:59:56',NULL,0),(27,25,20,2,'2026-02-24 21:02:10',NULL,0),(28,25,23,2,'2026-02-24 23:11:34',NULL,0),(29,25,24,4,'2026-02-25 00:12:27',NULL,0),(30,34,31,1,'2026-02-25 00:37:54',NULL,0),(31,25,22,1,'2026-02-27 01:02:01',NULL,0),(32,46,16,1,'2026-05-06 16:12:11',NULL,0),(34,31,61,1,'2026-06-19 19:38:52','2026-06-19 19:42:10',1),(35,1,91,1,'2026-06-20 01:35:23',NULL,0),(36,4,91,1,'2026-06-20 01:35:23',NULL,0),(37,27,92,1,'2026-06-20 01:35:23',NULL,0),(38,25,93,1,'2026-06-20 01:35:23',NULL,0),(39,46,93,1,'2026-06-20 01:35:23',NULL,0),(40,31,94,1,'2026-06-20 01:35:23',NULL,0),(41,33,94,1,'2026-06-20 01:35:23',NULL,0),(42,23,95,1,'2026-06-20 01:35:23',NULL,0),(43,45,95,1,'2026-06-20 01:35:23',NULL,0),(44,24,96,1,'2026-06-20 01:35:23',NULL,0),(45,30,96,1,'2026-06-20 01:35:23',NULL,0),(46,32,96,1,'2026-06-20 01:35:23',NULL,0),(47,34,96,1,'2026-06-20 01:35:23',NULL,0),(48,44,96,1,'2026-06-20 01:35:23',NULL,0),(49,49,96,1,'2026-06-20 01:35:23',NULL,0),(50,27,47,2,'2026-06-22 02:42:09',NULL,0),(51,27,48,1,'2026-06-22 02:42:25',NULL,0),(52,27,49,3,'2026-06-22 02:42:35',NULL,0),(53,27,50,1,'2026-06-22 02:42:49',NULL,0),(54,27,51,1,'2026-06-22 02:42:56',NULL,0),(55,27,52,1,'2026-06-22 02:43:03',NULL,0),(56,27,53,1,'2026-06-22 02:43:09',NULL,0),(57,27,54,1,'2026-06-22 02:43:16',NULL,0),(58,27,55,1,'2026-06-22 02:43:25',NULL,0),(59,27,56,1,'2026-06-22 02:43:34',NULL,0),(60,27,57,1,'2026-06-22 02:43:43',NULL,0),(61,27,58,1,'2026-06-22 02:43:50',NULL,0),(62,27,59,1,'2026-06-22 02:43:57',NULL,0),(63,27,60,5,'2026-06-22 02:44:02',NULL,0),(72,23,77,1,'2026-06-24 18:21:37',NULL,0),(73,32,32,5,'2026-06-27 13:51:25',NULL,0),(74,32,33,1,'2026-06-27 13:51:34',NULL,0),(75,32,34,1,'2026-06-27 13:51:42',NULL,0),(76,32,35,1,'2026-06-27 13:51:50',NULL,0),(77,32,36,1,'2026-06-27 13:51:55',NULL,0),(78,32,37,1,'2026-06-27 13:52:01',NULL,0),(79,32,38,1,'2026-06-27 13:52:09',NULL,0),(80,32,39,2,'2026-06-27 13:52:15',NULL,0),(81,32,40,1,'2026-06-27 13:52:24',NULL,0),(82,32,41,1,'2026-06-27 13:52:33',NULL,0),(83,32,42,1,'2026-06-27 13:52:40',NULL,0),(84,32,43,1,'2026-06-27 13:52:46',NULL,0),(85,32,44,1,'2026-06-27 13:52:53',NULL,0),(86,32,45,1,'2026-06-27 13:52:59',NULL,0),(93,45,76,1,'2026-07-03 22:58:16',NULL,0),(94,62,94,1,'2026-07-04 00:25:44',NULL,0),(95,62,61,1,'2026-07-04 00:26:22',NULL,0),(96,62,62,1,'2026-07-04 00:31:35',NULL,0),(97,62,63,1,'2026-07-04 00:38:41',NULL,0),(98,62,64,1,'2026-07-04 00:38:48',NULL,0),(99,62,65,1,'2026-07-04 00:38:57',NULL,0),(100,62,66,1,'2026-07-04 00:39:05',NULL,0),(101,62,68,1,'2026-07-04 00:52:14',NULL,0),(104,64,92,1,'2026-07-04 14:59:46',NULL,0),(105,64,46,1,'2026-07-04 15:00:58',NULL,0),(107,65,94,1,'2026-07-05 22:25:42',NULL,0),(108,66,92,1,'2026-07-06 18:05:17',NULL,0),(109,66,46,1,'2026-07-06 18:06:09',NULL,0);
/*!40000 ALTER TABLE `player_skills` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_stat_points`
--

DROP TABLE IF EXISTS `player_stat_points`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_stat_points` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `unspent_points` int unsigned DEFAULT '0' COMMENT 'Pontos não distribuídos',
  `strength_points` int unsigned DEFAULT '0' COMMENT 'Pontos adicionais em Strength (além do base da classe)',
  `dexterity_points` int unsigned DEFAULT '0' COMMENT 'Pontos adicionais em Dexterity',
  `intelligence_points` int unsigned DEFAULT '0' COMMENT 'Pontos adicionais em Intelligence',
  `vitality_points` int unsigned DEFAULT '0' COMMENT 'Pontos adicionais em Vitality',
  `luck_points` int unsigned DEFAULT '0' COMMENT 'Pontos adicionais em Luck',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_player_stat_points` (`player_id`),
  KEY `idx_player_id` (`player_id`),
  CONSTRAINT `player_stat_points_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=216 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_stat_points`
--

LOCK TABLES `player_stat_points` WRITE;
/*!40000 ALTER TABLE `player_stat_points` DISABLE KEYS */;
INSERT INTO `player_stat_points` VALUES (1,1,0,0,0,0,490,0,'2025-11-27 19:07:40','2026-07-01 04:46:35'),(4,2,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(5,4,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(6,6,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(7,14,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(8,11,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(9,17,0,0,0,0,0,0,'2025-11-27 19:07:40','2025-11-27 19:07:40'),(16,24,0,0,0,0,0,0,'2025-12-04 21:30:39','2025-12-04 21:30:39'),(17,25,0,0,0,0,0,0,'2025-12-04 22:04:40','2025-12-04 22:04:40'),(20,27,0,0,0,0,0,0,'2025-12-06 15:00:01','2025-12-06 15:00:01'),(22,23,100,0,0,0,0,0,'2025-12-08 20:51:17','2026-07-05 18:37:26'),(24,30,0,0,0,0,0,0,'2026-02-09 01:14:27','2026-02-09 01:14:27'),(25,31,0,0,0,0,0,0,'2026-02-23 15:25:37','2026-02-23 15:25:37'),(26,32,0,104,252,0,134,0,'2026-02-23 15:51:29','2026-07-01 05:01:57'),(27,33,0,0,0,0,0,0,'2026-02-24 19:29:21','2026-02-24 19:29:21'),(28,34,0,0,0,0,0,0,'2026-02-25 00:34:38','2026-02-25 00:34:38'),(38,44,0,0,0,0,0,0,'2026-04-19 07:30:25','2026-04-19 07:30:25'),(39,45,50,0,0,0,0,0,'2026-04-27 19:45:54','2026-06-30 17:28:37'),(40,46,0,0,0,0,0,0,'2026-05-06 16:08:29','2026-05-06 16:08:29'),(43,49,0,0,0,0,0,0,'2026-05-10 13:47:52','2026-05-10 13:47:52'),(169,62,118,0,0,132,0,0,'2026-07-04 00:25:48','2026-07-04 00:54:36'),(202,64,30,0,0,0,0,0,'2026-07-04 14:59:50','2026-07-04 15:01:47'),(207,65,10,0,0,0,0,0,'2026-07-05 22:25:46','2026-07-05 22:30:33'),(209,66,60,0,0,0,0,0,'2026-07-06 18:05:21','2026-07-06 18:06:36');
/*!40000 ALTER TABLE `player_stat_points` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_storage`
--

DROP TABLE IF EXISTS `player_storage`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_storage` (
  `storage_id` int NOT NULL AUTO_INCREMENT,
  `player_id` bigint unsigned NOT NULL,
  `inventory_id` int NOT NULL COMMENT 'Referência ao item em player_inventory',
  `slot_index` int NOT NULL COMMENT 'Índice do slot no storage (50-149)',
  `stored_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Quando o item foi armazenado',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`storage_id`),
  UNIQUE KEY `unique_player_storage_slot` (`player_id`,`slot_index`),
  UNIQUE KEY `unique_inventory_storage` (`inventory_id`,`player_id`),
  KEY `idx_player_storage` (`player_id`),
  KEY `idx_slot_index` (`player_id`,`slot_index`),
  CONSTRAINT `player_storage_ibfk_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `player_storage_ibfk_2` FOREIGN KEY (`inventory_id`) REFERENCES `player_inventory` (`inventory_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1446 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_storage`
--

LOCK TABLES `player_storage` WRITE;
/*!40000 ALTER TABLE `player_storage` DISABLE KEYS */;
INSERT INTO `player_storage` VALUES (1335,23,188,65,'2026-05-10 17:08:34','2026-05-10 20:09:07');
/*!40000 ALTER TABLE `player_storage` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `player_threat`
--

DROP TABLE IF EXISTS `player_threat`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `player_threat` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `mob_instance_id` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `player_id` bigint unsigned NOT NULL,
  `threat_value` int unsigned NOT NULL DEFAULT '0',
  `updated_at` timestamp(3) NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_mob_player` (`mob_instance_id`,`player_id`),
  KEY `idx_mob_threat` (`mob_instance_id`,`threat_value` DESC),
  KEY `fk_threat_player` (`player_id`),
  CONSTRAINT `fk_threat_player` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `player_threat`
--

LOCK TABLES `player_threat` WRITE;
/*!40000 ALTER TABLE `player_threat` DISABLE KEYS */;
/*!40000 ALTER TABLE `player_threat` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `players`
--

DROP TABLE IF EXISTS `players`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `players` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `account_id` bigint unsigned NOT NULL,
  `character_name` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `level` int unsigned DEFAULT '1',
  `experience` bigint unsigned DEFAULT '0',
  `next_level_exp` int unsigned DEFAULT '0' COMMENT 'EXP necessário para próximo nível (cache)',
  `pos_x` float DEFAULT '0',
  `pos_y` float DEFAULT '0',
  `pos_z` float DEFAULT '100',
  `current_zone` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT 'Zone_1',
  `health` int unsigned DEFAULT '100',
  `max_health` int unsigned DEFAULT '100',
  `mana` int unsigned DEFAULT '100',
  `is_dead` tinyint(1) NOT NULL DEFAULT '0',
  `last_death_at` timestamp NULL DEFAULT NULL,
  `max_mana` int unsigned DEFAULT '100',
  `stamina` int unsigned DEFAULT '100',
  `max_stamina` int unsigned DEFAULT '100',
  `strength` int unsigned DEFAULT '10',
  `dexterity` int unsigned DEFAULT '10',
  `intelligence` int unsigned DEFAULT '10',
  `vitality` int unsigned DEFAULT '10',
  `hair` int unsigned DEFAULT '0' COMMENT 'ID do cabelo do personagem',
  `head` int unsigned DEFAULT '0' COMMENT 'ID da cabeça do personagem',
  `class_id` bigint unsigned DEFAULT NULL,
  `faction_id` bigint unsigned DEFAULT NULL,
  `current_guild_id` bigint unsigned DEFAULT NULL,
  `equipped_title_id` bigint unsigned DEFAULT NULL,
  `selected_class` bigint unsigned DEFAULT NULL,
  `luck` int unsigned DEFAULT '10',
  `pvp` int unsigned DEFAULT '0',
  `chaos` int unsigned DEFAULT '0',
  `honor` int unsigned DEFAULT '0',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_played_at` timestamp NULL DEFAULT NULL,
  `gold` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Quantidade de gold que o jogador possui',
  PRIMARY KEY (`id`),
  UNIQUE KEY `character_name` (`character_name`),
  KEY `idx_account` (`account_id`),
  KEY `idx_character_name` (`character_name`),
  KEY `idx_player_class` (`class_id`),
  KEY `idx_player_faction` (`faction_id`),
  KEY `idx_player_guild` (`current_guild_id`),
  KEY `idx_player_title` (`equipped_title_id`),
  KEY `idx_player_selected_class` (`selected_class`),
  KEY `idx_player_current_guild` (`current_guild_id`),
  CONSTRAINT `fk_player_class` FOREIGN KEY (`class_id`) REFERENCES `classes` (`class_id`) ON DELETE SET NULL,
  CONSTRAINT `fk_player_faction` FOREIGN KEY (`faction_id`) REFERENCES `factions` (`faction_id`) ON DELETE SET NULL,
  CONSTRAINT `fk_player_guild` FOREIGN KEY (`current_guild_id`) REFERENCES `guilds` (`guild_id`) ON DELETE SET NULL,
  CONSTRAINT `fk_player_selected_class` FOREIGN KEY (`selected_class`) REFERENCES `classes` (`class_id`) ON DELETE SET NULL,
  CONSTRAINT `fk_player_title` FOREIGN KEY (`equipped_title_id`) REFERENCES `titles` (`title_id`) ON DELETE SET NULL,
  CONSTRAINT `fk_players_current_guild` FOREIGN KEY (`current_guild_id`) REFERENCES `guilds` (`guild_id`) ON DELETE SET NULL,
  CONSTRAINT `players_ibfk_1` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=67 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `players`
--

LOCK TABLES `players` WRITE;
/*!40000 ALTER TABLE `players` DISABLE KEYS */;
INSERT INTO `players` VALUES (1,4,'ElJeffo',50,7606400,0,791.77,-1035.33,72.5276,'Tutorial',7490,150,5754,0,'2026-07-04 01:14:28',30,120,120,20,12,8,18,0,0,1,1,6,5,1,10,50,1,1000,'2025-10-16 18:32:23','2026-07-06 18:04:44',116405),(2,9,'Willzao',1,0,1000,-3274.2,-3067.82,128.072,'Tutorial',150,100,50,0,'2026-06-19 19:32:49',50,100,100,10,10,10,10,0,0,NULL,NULL,4,NULL,NULL,10,0,0,0,'2025-10-17 00:25:23','2026-06-21 01:06:58',9900),(4,9,'Spacce',1,0,1000,-150,-570,200,'Tutorial',200,100,50,0,NULL,50,100,100,10,10,10,10,0,0,1,1,NULL,NULL,1,10,50,1,1000,'2025-10-18 23:41:31','2026-06-19 19:40:07',0),(6,23,'Sonylover',1,0,1000,0,0,0,'Tutorial',100,100,50,0,NULL,50,100,100,10,10,10,10,0,0,NULL,NULL,NULL,NULL,NULL,10,0,0,0,'2025-10-19 00:54:46',NULL,0),(11,26,'devjeffo',1,0,1000,0,0,0,'Tutorial',100,100,50,0,NULL,50,100,100,10,10,10,10,0,0,NULL,NULL,NULL,NULL,NULL,10,0,0,0,'2025-10-29 23:04:26','2025-10-29 23:04:35',0),(14,25,'JourneyOfHeroes',1,0,1000,-150.002,766.671,92.15,'Tutorial',100,100,50,0,NULL,50,100,100,10,10,10,10,0,0,NULL,NULL,NULL,NULL,NULL,10,0,0,0,'2025-10-30 16:44:48','2026-02-20 01:10:30',0),(17,28,'jeffoso',1,0,1000,0,0,0,'Tutorial',100,100,50,0,NULL,50,100,100,10,10,10,10,0,0,NULL,NULL,NULL,NULL,NULL,10,0,0,0,'2025-11-01 02:09:00','2025-11-01 02:09:04',0),(23,29,'TheKillZone',11,23000,4048,-2881.14,-2548.73,120.311,'Tutorial',418,100,290,0,'2026-06-23 02:11:47',40,130,130,12,22,10,10,4,2,5,NULL,4,4,5,18,0,0,0,'2025-12-04 21:19:34','2026-07-05 18:38:17',9475890),(24,30,'JeffusMonk',1,0,1000,-132.001,625.659,92.15,'Tutorial',130,130,70,0,NULL,70,140,140,14,18,12,16,5,6,6,NULL,NULL,NULL,6,14,0,0,0,'2025-12-04 21:30:04','2025-12-04 21:32:47',0),(25,9,'MaguWill',30,5150,57662,-3448.86,2094.88,483.791,'Tutorial',710,80,810,0,'2026-06-22 12:45:00',150,90,90,8,10,22,10,5,3,3,NULL,4,NULL,3,12,0,0,0,'2025-12-04 22:04:26','2026-07-04 15:05:13',94260),(27,4,'JeffoTemplar',50,1650,0,-4068.23,961.343,480.839,'Tutorial',1605,180,2745,0,NULL,60,110,110,15,10,12,20,4,2,2,NULL,4,NULL,2,12,0,0,0,'2025-12-06 01:06:05','2026-07-05 18:23:53',10120100),(30,27,'ShaolimMataPorco',1,0,1000,-232.713,236.696,92,'Tutorial',130,130,70,0,NULL,70,140,140,14,18,12,16,1,1,6,NULL,NULL,NULL,6,14,0,0,0,'2026-02-09 01:14:16','2026-02-09 01:15:29',0),(31,9,'LoAlquimico',1,0,1000,-2580,-980,140,'1',120,120,140,0,'2026-06-19 19:46:13',140,100,100,10,10,18,15,2,1,4,NULL,NULL,NULL,4,15,0,0,0,'2026-02-23 15:25:27','2026-06-19 19:46:13',0),(32,29,'LoMartial',50,7589500,0,-932.237,-354.34,88.5485,'Tutorial',3717,130,2971,0,'2026-07-04 01:16:39',70,140,140,14,18,12,16,1,1,6,NULL,NULL,NULL,6,14,0,0,0,'2026-02-23 15:51:21','2026-07-04 01:17:41',0),(33,25,'Test001',1,0,1000,-2647.28,-1600,99.0695,'Tutorial',120,120,140,0,NULL,140,100,100,10,10,18,15,1,1,4,NULL,NULL,NULL,4,15,0,0,0,'2026-02-24 19:29:12','2026-04-14 18:09:24',0),(34,31,'ZeCuzero',1,0,1000,523.924,957.334,92.15,'Tutorial',130,130,70,0,NULL,70,140,140,14,18,12,16,1,1,6,NULL,NULL,NULL,6,14,0,0,0,'2026-02-25 00:34:26','2026-02-25 00:38:39',0),(44,32,'JuniorPREDADOR',1,0,1000,-320,710,90.561,'Tutorial',130,130,70,0,NULL,70,140,140,14,18,12,16,1,1,6,NULL,NULL,NULL,6,14,0,0,0,'2026-04-19 07:30:06','2026-04-19 07:37:48',0),(45,29,'asss',6,7150,2013,1097.48,-1951.68,58.7629,'Tutorial',250,100,190,0,NULL,40,130,130,12,22,10,10,1,1,5,NULL,NULL,NULL,5,18,0,0,0,'2026-04-27 19:12:05','2026-07-03 23:01:16',100),(46,34,'joao',1,0,1000,-1332.19,-294.944,93.4778,'Tutorial',80,80,150,0,NULL,150,90,90,8,10,22,10,2,1,3,NULL,NULL,NULL,3,12,0,0,0,'2026-05-06 16:08:17','2026-05-08 00:56:29',44999),(49,35,'MongerSS',1,0,1000,-607.604,147.267,89.4422,'Tutorial',130,130,70,0,NULL,70,140,140,14,18,12,16,1,1,6,NULL,NULL,NULL,6,14,0,0,0,'2026-05-10 13:47:42','2026-05-10 13:48:21',0),(62,29,'Jeffu',26,230000,32969,1028.31,-1014.85,159.407,'Tutorial',670,120,1110,0,NULL,140,100,100,10,10,18,15,1,1,4,NULL,NULL,NULL,4,15,0,0,0,'2026-07-04 00:25:44','2026-07-04 01:00:57',0),(64,29,'aaaaa',4,3500,1521,1403.51,-3434.9,58.6795,'Tutorial',320,180,161,0,NULL,60,110,110,15,10,12,20,1,1,2,NULL,NULL,NULL,2,12,0,0,0,'2026-07-04 14:59:46','2026-07-04 15:01:58',0),(65,25,'QuestTestr',2,1000,1150,-625.033,-927.024,77.9429,'Tutorial',190,120,210,0,NULL,140,100,100,10,10,18,15,1,1,4,NULL,NULL,NULL,4,15,0,0,0,'2026-07-05 22:25:42','2026-07-05 22:36:14',0),(66,4,'Jef',7,10000,2315,1375.25,-3486.54,58.411,'Tutorial',380,180,194,0,NULL,60,110,110,15,10,12,20,1,1,2,NULL,NULL,NULL,2,12,0,0,0,'2026-07-06 18:05:17','2026-07-06 18:08:10',0);
/*!40000 ALTER TABLE `players` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_player_skills_grant_basic_attack` AFTER INSERT ON `players` FOR EACH ROW BEGIN
  DECLARE v_skill_id INT UNSIGNED DEFAULT NULL;
  SELECT s.skill_id INTO v_skill_id
    FROM skills s
   WHERE s.class_id = NEW.class_id
     AND s.is_basic_attack = 1
     AND s.is_enabled = 1
   ORDER BY s.skill_id ASC
   LIMIT 1;

  IF v_skill_id IS NOT NULL THEN
    INSERT IGNORE INTO player_skills (player_id, skill_id, current_rank)
    VALUES (NEW.id, v_skill_id, 1);
  END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_update_stat_points_on_level` AFTER UPDATE ON `players` FOR EACH ROW BEGIN
    DECLARE levels_gained INT;
    IF NEW.level > OLD.level THEN
        SET levels_gained = NEW.level - OLD.level;
        INSERT INTO player_stat_points (player_id, unspent_points)
        VALUES (NEW.id, levels_gained * 10)
        ON DUPLICATE KEY UPDATE
            unspent_points = unspent_points + (levels_gained * 10);
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `trg_update_skill_points_on_level` AFTER UPDATE ON `players` FOR EACH ROW BEGIN
    IF NEW.level > OLD.level THEN
        UPDATE player_skill_points
        SET total_points_earned = NEW.level * 3,
            points_available = (NEW.level * 3) - points_spent
        WHERE player_id = NEW.id;
    END IF;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `quest_objectives`
--

DROP TABLE IF EXISTS `quest_objectives`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `quest_objectives` (
  `objective_id` int unsigned NOT NULL AUTO_INCREMENT,
  `quest_id` int unsigned NOT NULL,
  `sort_order` int NOT NULL DEFAULT '0',
  `objective_type` enum('talk','kill','collect','deliver','reach_area','use_item_at') COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `params_json` json NOT NULL,
  PRIMARY KEY (`objective_id`),
  KEY `idx_quest_objectives_quest` (`quest_id`),
  CONSTRAINT `fk_quest_objectives_quest` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `quest_objectives`
--

LOCK TABLES `quest_objectives` WRITE;
/*!40000 ALTER TABLE `quest_objectives` DISABLE KEYS */;
INSERT INTO `quest_objectives` VALUES (1,1,0,'talk','Fale com o mercador','{}'),(2,1,1,'kill','Derrote bonecos de treino','{\"required_count\": 3, \"npc_template_id\": 6}'),(3,1,1,'kill','Derrote bonecos de treino','{\"required_count\": 3, \"npc_template_id\": 7}'),(5,2,0,'collect','Tenha poções na bolsa','{\"required_count\": 1, \"item_template_id\": 7}'),(6,2,1,'deliver','Entregue a poção ao mercador','{\"required_count\": 1, \"item_template_id\": 7}'),(7,3,0,'reach_area','Vá ao santuário','{\"pos_x\": 600.0, \"pos_y\": 0.0, \"pos_z\": 200.0, \"radius\": 400.0, \"zone_id\": 1}'),(8,3,1,'use_item_at','Use o Medalhão Antigo no santuário','{\"pos_x\": 600.0, \"pos_y\": 0.0, \"pos_z\": 200.0, \"radius\": 400.0, \"zone_id\": 1, \"item_template_id\": 1}'),(9,4,1,'kill','Mate os Goblins.','{\"required_count\": 10, \"npc_template_id\": 10}');
/*!40000 ALTER TABLE `quest_objectives` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `quest_reward_choices`
--

DROP TABLE IF EXISTS `quest_reward_choices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `quest_reward_choices` (
  `choice_id` int unsigned NOT NULL AUTO_INCREMENT,
  `quest_id` int unsigned NOT NULL,
  `choice_group_id` int unsigned NOT NULL DEFAULT '1',
  `label` varchar(120) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `reward_type` enum('gold','experience','item') COLLATE utf8mb4_unicode_ci NOT NULL,
  `amount` int NOT NULL DEFAULT '0',
  `item_template_id` int DEFAULT NULL,
  `quantity` int NOT NULL DEFAULT '1',
  `sort_order` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`choice_id`),
  KEY `idx_quest_reward_choices_quest` (`quest_id`),
  CONSTRAINT `fk_quest_reward_choices_quest` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `quest_reward_choices`
--

LOCK TABLES `quest_reward_choices` WRITE;
/*!40000 ALTER TABLE `quest_reward_choices` DISABLE KEYS */;
INSERT INTO `quest_reward_choices` VALUES (1,3,1,'100 Gold','gold',100,NULL,1,0),(2,3,1,'Poção extra','item',0,7,3,1);
/*!40000 ALTER TABLE `quest_reward_choices` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `quest_rewards`
--

DROP TABLE IF EXISTS `quest_rewards`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `quest_rewards` (
  `reward_id` int unsigned NOT NULL AUTO_INCREMENT,
  `quest_id` int unsigned NOT NULL,
  `reward_type` enum('gold','experience','item') COLLATE utf8mb4_unicode_ci NOT NULL,
  `amount` int NOT NULL DEFAULT '0',
  `item_template_id` int DEFAULT NULL,
  `quantity` int NOT NULL DEFAULT '1',
  `choice_group_id` int unsigned DEFAULT NULL,
  `sort_order` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`reward_id`),
  KEY `idx_quest_rewards_quest` (`quest_id`),
  CONSTRAINT `fk_quest_rewards_quest` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `quest_rewards`
--

LOCK TABLES `quest_rewards` WRITE;
/*!40000 ALTER TABLE `quest_rewards` DISABLE KEYS */;
INSERT INTO `quest_rewards` VALUES (1,1,'gold',100,NULL,1,NULL,0),(2,1,'experience',150,NULL,1,NULL,1),(3,2,'item',0,7,2,NULL,0),(4,4,'item',1,1,1,NULL,0);
/*!40000 ALTER TABLE `quest_rewards` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `quests`
--

DROP TABLE IF EXISTS `quests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `quests` (
  `quest_id` int unsigned NOT NULL AUTO_INCREMENT,
  `quest_key` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `title` varchar(120) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` text COLLATE utf8mb4_unicode_ci,
  `offer_text` text COLLATE utf8mb4_unicode_ci,
  `turn_in_text` text COLLATE utf8mb4_unicode_ci,
  `min_level` int unsigned NOT NULL DEFAULT '1',
  `prerequisite_quest_id` int unsigned DEFAULT NULL,
  `repeatable` tinyint(1) NOT NULL DEFAULT '0',
  `turn_in_npc_template_id` bigint unsigned DEFAULT NULL,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`quest_id`),
  UNIQUE KEY `uk_quests_key` (`quest_key`),
  KEY `idx_quests_prereq` (`prerequisite_quest_id`),
  CONSTRAINT `fk_quests_prereq` FOREIGN KEY (`prerequisite_quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `quests`
--

LOCK TABLES `quests` WRITE;
/*!40000 ALTER TABLE `quests` DISABLE KEYS */;
INSERT INTO `quests` VALUES (1,'quest_merchant_kill_training','Treino de Combate','Derrote alvos de treino para provar sua força.','Preciso que você derrote alguns bonecos de treino. Volte quando terminar.','Excelente trabalho! Aqui está sua recompensa.',1,NULL,1,5,1,'2026-07-03 15:56:18'),(2,'quest_merchant_deliver_herb','Ervas para o Mercador','Colete e entregue ervas medicinais.','Traga-me ervas medicinais da sua bolsa. Eu pago bem.','Obrigado pelas ervas!',1,NULL,0,5,1,'2026-07-03 15:56:18'),(3,'quest_merchant_shrine','O Santuário Esquecido','Visite o santuário e use o medalhão antigo.','Há um santuário próximo. Vá até lá e use o medalhão antigo.','Você desvendou o mistério do santuário!',1,2,0,5,1,'2026-07-03 15:56:18'),(4,'quest_citizen_village_help','Vingue o cidadão da vila!','O cidadão da vila está desolado por ter perdido seu grande amor em um ataque goblin. Elimine os goblins para concretizar a sua vingança.','Os malditos Goblins que mataram minha esposa estão circulando pelas redondezas, você irá eliminá-los?','Obrigado por eliminar os Goblins! hoje, finalmente poderei dormir em paz.',1,1,1,9,1,'2026-07-04 03:54:19');
/*!40000 ALTER TABLE `quests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `refinement_config`
--

DROP TABLE IF EXISTS `refinement_config`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `refinement_config` (
  `refinement_level` tinyint unsigned NOT NULL COMMENT 'Nível de refinação atual (+0 a +12)',
  `success_rate` float NOT NULL COMMENT 'Taxa de sucesso (0.0 a 1.0)',
  `required_item_id` int NOT NULL COMMENT 'ID do item de refinação necessário',
  `required_item_quantity` int NOT NULL DEFAULT '1' COMMENT 'Quantidade do item necessária',
  `stat_bonus_multiplier` float NOT NULL COMMENT 'Multiplicador de stats (ex: 1.05 = +5%, 1.10 = +10%)',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`refinement_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `refinement_config`
--

LOCK TABLES `refinement_config` WRITE;
/*!40000 ALTER TABLE `refinement_config` DISABLE KEYS */;
INSERT INTO `refinement_config` VALUES (0,1,52,1,1,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(1,1,52,1,1.05,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(2,1,52,2,1.1,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(3,1,52,2,1.15,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(4,1,52,3,1.2,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(5,1,52,3,1.25,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(6,1,52,4,1.3,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(7,0.75,53,1,1.35,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(8,0.6,53,2,1.4,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(9,0.45,53,3,1.45,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(10,0.3,53,4,1.5,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(11,0.2,53,5,1.55,'2026-04-27 21:24:10','2026-04-27 21:24:10'),(12,0,53,0,1.6,'2026-04-27 21:24:10','2026-04-27 21:24:10');
/*!40000 ALTER TABLE `refinement_config` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `schema_version`
--

DROP TABLE IF EXISTS `schema_version`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `schema_version` (
  `version` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `applied_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `schema_version`
--

LOCK TABLES `schema_version` WRITE;
/*!40000 ALTER TABLE `schema_version` DISABLE KEYS */;
INSERT INTO `schema_version` VALUES ('1.3.0','2025-10-14 16:51:19');
/*!40000 ALTER TABLE `schema_version` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_effects`
--

DROP TABLE IF EXISTS `skill_effects`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_effects` (
  `effect_id` int unsigned NOT NULL AUTO_INCREMENT,
  `skill_id` int unsigned NOT NULL,
  `effect_order` tinyint unsigned NOT NULL DEFAULT '1',
  `effect_type` enum('DAMAGE','HEAL','SHIELD','BUFF_STAT','DEBUFF_STAT','DOT','HOT','CLEANSE','DISPEL','STUN','SILENCE','SLOW','ROOT','KNOCKBACK','TAUNT','STEALTH','INVULNERABLE','LIFESTEAL','MANASTEAL','SUMMON','TELEPORT','EXECUTE','REFLECT','COOLDOWN_RESET','RESOURCE_RESTORE') COLLATE utf8mb4_unicode_ci NOT NULL,
  `target_stat` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `value_flat` int NOT NULL DEFAULT '0',
  `value_percent` smallint NOT NULL DEFAULT '0',
  `duration_ms` int unsigned NOT NULL DEFAULT '0',
  `tick_interval_ms` int unsigned NOT NULL DEFAULT '1000',
  `chance_percent` tinyint unsigned NOT NULL DEFAULT '100',
  `conditions_json` json DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`effect_id`),
  KEY `idx_skill_effects` (`skill_id`,`effect_order`),
  CONSTRAINT `fk_effects_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_effects`
--

LOCK TABLES `skill_effects` WRITE;
/*!40000 ALTER TABLE `skill_effects` DISABLE KEYS */;
/*!40000 ALTER TABLE `skill_effects` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_elements`
--

DROP TABLE IF EXISTS `skill_elements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_elements` (
  `element_id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `element_name` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `element_key` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `color_hex` varchar(7) COLLATE utf8mb4_unicode_ci DEFAULT '#FFFFFF',
  `description` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`element_id`),
  UNIQUE KEY `uk_element_key` (`element_key`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_elements`
--

LOCK TABLES `skill_elements` WRITE;
/*!40000 ALTER TABLE `skill_elements` DISABLE KEYS */;
INSERT INTO `skill_elements` VALUES (1,'Physical','PHYSICAL','#C0C0C0','Dano físico bruto','2026-02-21 21:32:40'),(2,'Shadow','SHADOW','#4B0082','Energia dimensional da Neblina Vermelha','2026-02-21 21:32:40'),(3,'Fire','FIRE','#FF4500','Chamas e calor intenso','2026-02-21 21:32:40'),(4,'Holy','HOLY','#FFD700','Energia sagrada purificadora','2026-02-21 21:32:40'),(5,'Poison','POISON','#32CD32','Toxinas e venenos','2026-02-21 21:32:40'),(6,'Ice','ICE','#00BFFF','Gelo e frio congelante','2026-02-21 21:32:40'),(7,'Lightning','LIGHTNING','#FFFF00','Eletricidade e raios','2026-02-21 21:32:40'),(8,'Arcane','ARCANE','#9400D3','Magia pura dimensional','2026-02-21 21:32:40');
/*!40000 ALTER TABLE `skill_elements` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_points_per_level`
--

DROP TABLE IF EXISTS `skill_points_per_level`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_points_per_level` (
  `level` tinyint unsigned NOT NULL,
  `points_granted` tinyint unsigned NOT NULL DEFAULT '3',
  `cumulative_points` int unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_points_per_level`
--

LOCK TABLES `skill_points_per_level` WRITE;
/*!40000 ALTER TABLE `skill_points_per_level` DISABLE KEYS */;
INSERT INTO `skill_points_per_level` VALUES (1,3,3),(2,3,6),(3,3,9),(4,3,12),(5,3,15),(6,3,18),(7,3,21),(8,3,24),(9,3,27),(10,3,30),(11,3,33),(12,3,36),(13,3,39),(14,3,42),(15,3,45),(16,3,48),(17,3,51),(18,3,54),(19,3,57),(20,3,60),(21,3,63),(22,3,66),(23,3,69),(24,3,72),(25,3,75),(26,3,78),(27,3,81),(28,3,84),(29,3,87),(30,3,90),(31,3,93),(32,3,96),(33,3,99),(34,3,102),(35,3,105),(36,3,108),(37,3,111),(38,3,114),(39,3,117),(40,3,120),(41,3,123),(42,3,126),(43,3,129),(44,3,132),(45,3,135),(46,3,138),(47,3,141),(48,3,144),(49,3,147),(50,3,150);
/*!40000 ALTER TABLE `skill_points_per_level` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_rank_scaling`
--

DROP TABLE IF EXISTS `skill_rank_scaling`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_rank_scaling` (
  `scaling_id` int unsigned NOT NULL AUTO_INCREMENT,
  `skill_id` int unsigned NOT NULL,
  `rank` tinyint unsigned NOT NULL,
  `power_coef_bonus` smallint NOT NULL DEFAULT '0',
  `resource_cost_bonus` smallint NOT NULL DEFAULT '0',
  `cooldown_reduction_ms` int NOT NULL DEFAULT '0',
  `duration_bonus_ms` int NOT NULL DEFAULT '0',
  `extra_effects_json` json DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`scaling_id`),
  UNIQUE KEY `uk_skill_rank` (`skill_id`,`rank`),
  CONSTRAINT `fk_rank_skill` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`skill_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_rank_scaling`
--

LOCK TABLES `skill_rank_scaling` WRITE;
/*!40000 ALTER TABLE `skill_rank_scaling` DISABLE KEYS */;
/*!40000 ALTER TABLE `skill_rank_scaling` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_scaling_stats`
--

DROP TABLE IF EXISTS `skill_scaling_stats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_scaling_stats` (
  `scaling_id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `scaling_name` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `scaling_key` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`scaling_id`),
  UNIQUE KEY `uk_scaling_key` (`scaling_key`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_scaling_stats`
--

LOCK TABLES `skill_scaling_stats` WRITE;
/*!40000 ALTER TABLE `skill_scaling_stats` DISABLE KEYS */;
INSERT INTO `skill_scaling_stats` VALUES (1,'Physical Attack','PHYS_ATK','Escala com ataque físico','2026-02-21 21:32:40'),(2,'Magic Attack','MAG_ATK','Escala com ataque mágico','2026-02-21 21:32:40'),(3,'Health','HEALTH','Escala com vida máxima','2026-02-21 21:32:40'),(4,'Defense','DEFENSE','Escala com defesa','2026-02-21 21:32:40'),(5,'None','NONE','Sem scaling de stat','2026-02-21 21:32:40');
/*!40000 ALTER TABLE `skill_scaling_stats` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_targets`
--

DROP TABLE IF EXISTS `skill_targets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_targets` (
  `target_id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `target_name` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `target_key` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`target_id`),
  UNIQUE KEY `uk_target_key` (`target_key`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_targets`
--

LOCK TABLES `skill_targets` WRITE;
/*!40000 ALTER TABLE `skill_targets` DISABLE KEYS */;
INSERT INTO `skill_targets` VALUES (1,'Self','SELF','Afeta apenas o próprio usuário','2026-02-21 21:32:40'),(2,'Enemy','ENEMY','Afeta um inimigo único','2026-02-21 21:32:40'),(3,'Ally','ALLY','Afeta um aliado único','2026-02-21 21:32:40'),(4,'Area','AREA','Afeta área (inimigos)','2026-02-21 21:32:40'),(5,'Party','PARTY','Afeta todo o grupo','2026-02-21 21:32:40'),(6,'Area Ally','AREA_ALLY','Afeta área (aliados)','2026-02-21 21:32:40');
/*!40000 ALTER TABLE `skill_targets` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skill_types`
--

DROP TABLE IF EXISTS `skill_types`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skill_types` (
  `type_id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `type_name` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `type_key` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` text COLLATE utf8mb4_unicode_ci,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`type_id`),
  UNIQUE KEY `uk_type_key` (`type_key`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skill_types`
--

LOCK TABLES `skill_types` WRITE;
/*!40000 ALTER TABLE `skill_types` DISABLE KEYS */;
INSERT INTO `skill_types` VALUES (1,'Active','ACTIVE','Habilidade ativa que requer ativação manual','2026-02-21 21:32:40'),(2,'Passive','PASSIVE','Habilidade passiva sempre ativa','2026-02-21 21:32:40'),(3,'Buff','BUFF','Buff que melhora stats próprios ou aliados','2026-02-21 21:32:40'),(4,'Debuff','DEBUFF','Debuff que reduz stats inimigos','2026-02-21 21:32:40'),(5,'Aura','AURA','Aura que afeta área ao redor continuamente','2026-02-21 21:32:40'),(6,'Ultimate','ULTIMATE','Habilidade ultimate poderosa','2026-02-21 21:32:40'),(7,'Reaction','REACTION','Habilidade reativa a eventos','2026-02-21 21:32:40'),(8,'DOT','DOT','Damage Over Time - dano contínuo','2026-02-21 21:32:40'),(9,'HOT','HOT','Heal Over Time - cura contínua','2026-02-21 21:32:40');
/*!40000 ALTER TABLE `skill_types` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `skills`
--

DROP TABLE IF EXISTS `skills`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `skills` (
  `skill_id` int unsigned NOT NULL AUTO_INCREMENT,
  `skill_key` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `skill_name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `class_id` bigint unsigned NOT NULL,
  `skill_order` tinyint unsigned NOT NULL DEFAULT '1',
  `required_level` tinyint unsigned NOT NULL DEFAULT '1',
  `skill_cost` tinyint unsigned NOT NULL DEFAULT '1',
  `max_rank` tinyint unsigned NOT NULL DEFAULT '5',
  `type_id` tinyint unsigned NOT NULL,
  `target_id` tinyint unsigned NOT NULL,
  `element_id` tinyint unsigned NOT NULL DEFAULT '1',
  `scaling_stat_id` tinyint unsigned NOT NULL DEFAULT '1',
  `str_scaling` tinyint unsigned NOT NULL DEFAULT '0',
  `dex_scaling` tinyint unsigned NOT NULL DEFAULT '0',
  `vit_scaling` tinyint unsigned NOT NULL DEFAULT '0',
  `int_scaling` tinyint unsigned NOT NULL DEFAULT '0',
  `lck_scaling` tinyint unsigned NOT NULL DEFAULT '0',
  `power_coef` smallint unsigned NOT NULL DEFAULT '100',
  `secondary_coef` smallint unsigned NOT NULL DEFAULT '0',
  `resource_type` enum('MANA','HEALTH','STAMINA','NONE') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'MANA',
  `resource_cost` smallint unsigned NOT NULL DEFAULT '0',
  `resource_cost_percent` tinyint unsigned NOT NULL DEFAULT '0',
  `cooldown_ms` int unsigned NOT NULL DEFAULT '0',
  `cast_time_ms` int unsigned NOT NULL DEFAULT '0',
  `duration_ms` int unsigned NOT NULL DEFAULT '0',
  `range_min` smallint unsigned NOT NULL DEFAULT '0',
  `range_max` smallint unsigned NOT NULL DEFAULT '100',
  `area_radius` smallint unsigned NOT NULL DEFAULT '0',
  `is_stackable` tinyint(1) NOT NULL DEFAULT '0',
  `max_stacks` tinyint unsigned NOT NULL DEFAULT '1',
  `can_crit` tinyint(1) NOT NULL DEFAULT '1',
  `ignores_defense` tinyint(1) NOT NULL DEFAULT '0',
  `is_interrupt` tinyint(1) NOT NULL DEFAULT '0',
  `requires_target` tinyint(1) NOT NULL DEFAULT '1',
  `can_move_while_casting` tinyint(1) NOT NULL DEFAULT '0',
  `threat_modifier` smallint NOT NULL DEFAULT '100',
  `pvp_modifier` tinyint unsigned NOT NULL DEFAULT '100',
  `icon_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `vfx_key` varchar(100) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `sfx_key` varchar(100) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `cast_anim_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `vfx_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `sfx_path` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `hit_window_ms` int unsigned NOT NULL DEFAULT '300',
  `damage_type` enum('PHYSICAL','MAGIC','TRUE') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'PHYSICAL',
  `description` text COLLATE utf8mb4_unicode_ci,
  `tooltip_template` text COLLATE utf8mb4_unicode_ci,
  `server_tags` json DEFAULT NULL,
  `effects_json` json DEFAULT NULL,
  `is_enabled` tinyint(1) NOT NULL DEFAULT '1',
  `is_basic_attack` tinyint(1) NOT NULL DEFAULT '0',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`skill_id`),
  UNIQUE KEY `uk_skill_key` (`skill_key`),
  KEY `idx_class_id` (`class_id`),
  KEY `idx_type_id` (`type_id`),
  KEY `idx_required_level` (`required_level`),
  KEY `idx_class_order` (`class_id`,`skill_order`),
  KEY `fk_skills_target` (`target_id`),
  KEY `fk_skills_element` (`element_id`),
  KEY `fk_skills_scaling` (`scaling_stat_id`),
  KEY `idx_class_basic` (`class_id`,`is_basic_attack`),
  CONSTRAINT `fk_skills_class` FOREIGN KEY (`class_id`) REFERENCES `classes` (`class_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_element` FOREIGN KEY (`element_id`) REFERENCES `skill_elements` (`element_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_scaling` FOREIGN KEY (`scaling_stat_id`) REFERENCES `skill_scaling_stats` (`scaling_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_target` FOREIGN KEY (`target_id`) REFERENCES `skill_targets` (`target_id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  CONSTRAINT `fk_skills_type` FOREIGN KEY (`type_id`) REFERENCES `skill_types` (`type_id`) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=97 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `skills`
--

LOCK TABLES `skills` WRITE;
/*!40000 ALTER TABLE `skills` DISABLE KEYS */;
INSERT INTO `skills` VALUES (1,'BARB_RUIN_STRIKE','Golpe da Ruína',1,1,1,1,5,1,2,1,1,60,0,0,0,0,180,25,'MANA',15,0,3000,500,0,0,150,0,0,1,1,0,0,1,0,110,100,'Skills/Barbarian/T_Ruin_Strike','VFX_Heavy_Slash','SFX_Heavy_Impact',NULL,NULL,NULL,300,'PHYSICAL','Um ataque descendente brutal que quebra defesas pela força pura.','Causa {damage} de dano físico. {secondary}% de chance de Double Attack.','[\"melee\", \"single_target\", \"high_damage\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 180}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 0, \"target_stat\": \"double_attack_chance\", \"value_percent\": 25, \"chance_percent\": 100}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(2,'BARB_HARDENED_SKIN','Pele Endurecida',1,2,4,1,5,2,1,1,4,0,0,50,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Hardened_Skin',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','A carne calejada absorve impacto como couro de guerra.','+{value} Defesa Física permanente.','[\"passive\", \"defensive\", \"permanent\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 50, \"target_stat\": \"physical_defense\", \"value_percent\": 15}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(3,'BARB_WILD_CHARGE','Investida Selvagem',1,3,7,1,5,1,2,1,1,40,20,0,0,0,140,0,'MANA',25,0,8000,0,3000,0,300,0,0,1,1,0,0,1,1,100,100,'Skills/Barbarian/T_Wild_Charge','VFX_Charge','SFX_Rush',NULL,NULL,NULL,300,'PHYSICAL','A bárbara avança como uma avalanche viva.','Avança até o alvo causando {damage} de dano. +{speed}% Movement Speed por {duration}s.','[\"melee\", \"gap_closer\", \"mobility\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 140}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 3000, \"target_stat\": \"movement_speed\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(4,'BARB_HOT_BLOOD','Sangue Quente',1,4,10,1,5,2,1,1,1,0,0,0,0,40,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Hot_Blood',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Ferimentos despertam a fúria latente.','+{value}% Critical quando Health abaixo de 30%.','[\"passive\", \"offensive\", \"low_health_trigger\"]','[{\"type\": \"BUFF_STAT\", \"target_stat\": \"critical_chance\", \"value_percent\": 25, \"conditions_json\": {\"health_below_percent\": 30}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(5,'BARB_WAR_CRY','Rugido de Guerra',1,5,14,1,5,3,5,1,1,50,0,0,0,0,0,0,'MANA',40,0,30000,1000,15000,0,0,500,0,1,0,0,0,0,0,150,100,'Skills/Barbarian/T_War_Cry','VFX_War_Shout','SFX_Roar',NULL,NULL,NULL,300,'PHYSICAL','Um grito que endurece a coragem dos vivos.','+{value}% Phys Atk para aliados próximos por {duration}s.','[\"buff\", \"party\", \"aoe_ally\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"physical_attack\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(6,'BARB_IRON_BONES','Ossos de Ferro',1,6,18,1,5,2,1,1,4,0,0,45,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Iron_Bones',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Seus ossos foram quebrados — e reconstruídos mais fortes.','+{value} Critical Resistance permanente.','[\"passive\", \"defensive\", \"anti_crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"target_stat\": \"critical_resistance\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(7,'BARB_RENDING_CUT','Corte Dilacerante',1,7,22,1,5,1,2,1,1,55,0,0,0,0,130,40,'MANA',30,0,6000,300,8000,0,150,0,1,3,1,0,0,1,0,100,100,'Skills/Barbarian/T_Rending_Cut','VFX_Slash_Bleed','SFX_Slash',NULL,NULL,NULL,300,'PHYSICAL','A lâmina rasga carne e esperança.','Causa {damage} de dano e aplica Sangramento: {dot} dano/s por {duration}s.','[\"melee\", \"dot\", \"bleed\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 130}, {\"type\": \"DOT\", \"duration_ms\": 8000, \"target_stat\": \"health\", \"value_percent\": 40, \"tick_interval_ms\": 2000}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(8,'BARB_HUNTER_STEP','Passo da Caçadora',1,8,26,1,5,2,1,1,5,0,40,0,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Hunter_Step',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Movimento instintivo de predadora.','+{value} Dodge permanente.','[\"passive\", \"defensive\", \"evasion\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"target_stat\": \"dodge\", \"value_percent\": 8}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(9,'BARB_INSTINCT_RECOVERY','Recuperação Instintiva',1,9,30,1,5,1,1,1,3,0,0,60,0,0,0,0,'MANA',50,0,45000,1500,0,0,0,0,0,1,0,0,0,0,0,50,100,'Skills/Barbarian/T_Instinct_Recovery','VFX_Self_Heal','SFX_Breath',NULL,NULL,NULL,300,'PHYSICAL','Respiração profunda e dor ignorada.','Recupera {heal}% do Health máximo.','[\"heal\", \"self\", \"survival\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(10,'BARB_PRIMAL_RETALIATION','Retaliação Primitiva',1,10,34,1,5,7,1,1,1,50,0,0,0,0,120,0,'NONE',0,0,10000,0,0,0,150,0,0,1,1,0,0,0,0,100,100,'Skills/Barbarian/T_Primal_Retaliation','VFX_Counter','SFX_Counter_Hit',NULL,NULL,NULL,300,'PHYSICAL','Cada golpe sofrido cobra preço.','Ao receber crítico, contra-ataca causando {damage} de dano.','[\"reaction\", \"counter\", \"trigger_on_crit_received\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 120, \"conditions_json\": {\"trigger\": \"on_crit_received\"}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(11,'BARB_CRIMSON_FRENZY','Frenesi Carmesim',1,11,38,1,5,3,1,1,1,55,0,0,0,0,0,0,'MANA',35,0,25000,500,12000,0,0,0,0,1,0,0,0,0,0,120,100,'Skills/Barbarian/T_Crimson_Frenzy','VFX_Frenzy','SFX_Rage',NULL,NULL,NULL,300,'PHYSICAL','Troca proteção por massacre.','+{attack}% Double Attack, -{defense}% Defesa por {duration}s.','[\"buff\", \"self\", \"risk_reward\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 12000, \"target_stat\": \"double_attack_rate\", \"value_percent\": 35}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 12000, \"target_stat\": \"physical_defense\", \"value_percent\": -20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(12,'BARB_BRUTAL_RESISTANCE','Resistência Brutal',1,12,42,1,5,2,1,1,3,0,0,55,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Brutal_Resistance',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Sobrevive onde outros tombam.','+{value} Health Máximo permanente.','[\"passive\", \"defensive\", \"health\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 200, \"target_stat\": \"max_health\", \"value_percent\": 12}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(13,'BARB_CRUSHING_IMPACT','Impacto Esmagador',1,13,45,1,5,1,2,1,1,60,0,0,0,0,150,0,'MANA',40,0,12000,800,6000,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Barbarian/T_Crushing_Impact','VFX_Ground_Slam','SFX_Heavy_Impact',NULL,NULL,NULL,300,'PHYSICAL','Golpe que quebra postura.','Causa {damage} de dano e reduz Dodge do alvo em {debuff}% por {duration}s.','[\"melee\", \"debuff\", \"armor_break\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 150}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 6000, \"target_stat\": \"dodge\", \"value_percent\": -30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(14,'BARB_RELENTLESS_HUNT','Caçada Implacável',1,14,48,1,5,2,1,1,1,0,45,0,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Barbarian/T_Relentless_Hunt',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','A presa não escapa duas vezes.','+{value} Accuracy permanente.','[\"passive\", \"offensive\", \"accuracy\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 40, \"target_stat\": \"accuracy\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(15,'BARB_AVATAR_RUIN','Avatar da Ruína',1,15,50,1,5,6,1,1,1,50,0,30,0,20,0,0,'MANA',100,0,180000,2000,20000,0,0,0,0,1,0,0,0,0,0,200,80,'Skills/Barbarian/T_Avatar_Ruin','VFX_Ultimate_Barbarian','SFX_Ultimate_Roar',NULL,NULL,NULL,300,'PHYSICAL','A fúria vira entidade de guerra.','Por {duration}s: +{atk}% Phys Atk, +{speed}% Speed, +{double}% Double Attack.','[\"ultimate\", \"self_buff\", \"transformation\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"physical_attack\", \"value_percent\": 40}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"movement_speed\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"double_attack_rate\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(16,'DMAGE_CRIMSON_MIST','Rajada de Névoa Carmesim',3,1,1,1,5,1,2,2,2,0,0,0,60,0,170,0,'MANA',20,0,4000,800,0,0,400,0,0,1,1,0,0,1,0,100,100,'Skills/DarkMage/T_Crimson_Mist','VFX_Shadow_Bolt','SFX_Dark_Cast',NULL,NULL,NULL,300,'PHYSICAL','Dispara energia da Neblina Vermelha que corrói a alma.','Causa {damage} de dano Shadow.','[\"ranged\", \"magic\", \"shadow\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 170}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(17,'DMAGE_DIMENSIONAL_MARK','Marca Dimensional',3,2,4,1,5,4,2,2,2,0,0,0,55,0,0,0,'MANA',25,0,10000,600,10000,0,350,0,0,1,0,0,0,1,0,80,100,'Skills/DarkMage/T_Dimensional_Mark','VFX_Mark','SFX_Mark_Apply',NULL,NULL,NULL,300,'PHYSICAL','Selo que enfraquece o tecido mágico do inimigo.','Reduz Magic Defense do alvo em {value}% por {duration}s.','[\"debuff\", \"magic_pen\", \"control\"]','[{\"type\": \"DEBUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"magic_defense\", \"value_percent\": -25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(18,'DMAGE_SHADOW_VEIL','Véu Sombrio',3,3,7,1,5,3,1,2,5,0,40,0,0,0,0,0,'MANA',30,0,20000,500,8000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Shadow_Veil','VFX_Shadow_Aura','SFX_Shadow_Whisper',NULL,NULL,NULL,300,'PHYSICAL','O corpo oscila entre planos.','+{value}% Dodge por {duration}s.','[\"buff\", \"defensive\", \"evasion\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"dodge\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(19,'DMAGE_ARCANE_ECHO','Eco Arcano',3,4,10,1,5,2,1,8,2,0,0,0,0,45,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Arcane_Echo',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Magia reverbera além do primeiro impacto.','{value}% de chance de Double Magic Attack.','[\"passive\", \"offensive\", \"double_cast\"]','[{\"type\": \"BUFF_STAT\", \"target_stat\": \"double_attack_rate\", \"value_percent\": 15}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(20,'DMAGE_RUPTURE_SEAL','Selo de Ruptura',3,5,14,1,5,8,2,2,2,0,0,0,55,0,50,80,'MANA',35,0,8000,700,12000,0,350,0,1,3,0,0,0,1,0,90,100,'Skills/DarkMage/T_Rupture_Seal','VFX_DoT_Shadow','SFX_Seal',NULL,NULL,NULL,300,'PHYSICAL','Energia dimensional rasga lentamente.','Aplica {damage} de dano Shadow a cada {tick}s por {duration}s.','[\"dot\", \"shadow\", \"magic\"]','[{\"type\": \"DOT\", \"duration_ms\": 12000, \"target_stat\": \"health\", \"value_percent\": 80, \"tick_interval_ms\": 3000}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(21,'DMAGE_DEEP_MIND','Mente Profunda',3,6,18,1,5,2,1,8,5,0,0,0,50,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Deep_Mind',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Intelecto expandido pelo Véu.','+{value} Mana Máximo permanente.','[\"passive\", \"resource\", \"mana\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 150, \"target_stat\": \"max_mana\", \"value_percent\": 15}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(22,'DMAGE_CORRUPTION_ORB','Orbe da Corrupção',3,7,22,1,5,1,4,2,2,0,0,0,60,0,145,0,'MANA',45,0,10000,1200,0,0,350,200,0,1,1,0,0,1,0,100,100,'Skills/DarkMage/T_Corruption_Orb','VFX_AoE_Shadow','SFX_Explosion_Dark',NULL,NULL,NULL,300,'PHYSICAL','Explosão de energia carmesim instável.','Causa {damage} de dano Shadow em área.','[\"aoe\", \"magic\", \"shadow\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 145}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(23,'DMAGE_VEIL_REFLECTION','Reflexo do Véu',3,8,26,1,5,2,1,2,5,0,0,0,0,40,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Veil_Reflection',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Previsão arcana de impactos.','+{value} Critical Resistance permanente.','[\"passive\", \"defensive\", \"anti_crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"target_stat\": \"critical_resistance\", \"value_percent\": 8}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(24,'DMAGE_CRIMSON_DRAIN','Dreno Carmesim',3,9,30,1,5,1,2,2,2,0,0,0,55,0,120,40,'MANA',40,0,12000,800,0,0,300,0,0,1,1,0,0,1,0,100,100,'Skills/DarkMage/T_Crimson_Drain','VFX_Drain','SFX_Soul_Drain',NULL,NULL,NULL,300,'PHYSICAL','Rouba vitalidade pela ruptura planar.','Causa {damage} de dano e cura {heal}% do dano causado.','[\"magic\", \"lifesteal\", \"sustain\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 120}, {\"type\": \"LIFESTEAL\", \"value_percent\": 40}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(25,'DMAGE_RUBY_CONVERGENCE','Convergência Rubicina',3,10,34,1,5,3,1,8,2,0,0,0,50,0,0,0,'MANA',35,0,25000,600,12000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Ruby_Convergence','VFX_Magic_Buff','SFX_Power_Up',NULL,NULL,NULL,300,'PHYSICAL','Amplifica foco em cristal arcano.','+{value}% Magic Attack por {duration}s.','[\"buff\", \"offensive\", \"magic_power\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 12000, \"target_stat\": \"magic_attack\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(26,'DMAGE_REALITY_TWIST','Torção da Realidade',3,11,38,1,5,4,2,8,2,0,0,0,0,45,0,0,'MANA',30,0,15000,500,8000,0,350,0,0,1,0,0,0,1,0,80,100,'Skills/DarkMage/T_Reality_Twist','VFX_Distortion','SFX_Warp',NULL,NULL,NULL,300,'PHYSICAL','Distorce percepção inimiga.','Reduz Accuracy do alvo em {value}% por {duration}s.','[\"debuff\", \"control\", \"blind\"]','[{\"type\": \"DEBUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"accuracy\", \"value_percent\": -25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(27,'DMAGE_RIFT_STEP','Passo Entre Fendas',3,12,42,1,5,1,1,8,5,0,40,0,0,0,0,0,'MANA',25,0,15000,0,0,0,0,0,0,1,0,0,0,0,1,100,100,'Skills/DarkMage/T_Rift_Step','VFX_Teleport','SFX_Blink',NULL,NULL,NULL,300,'PHYSICAL','Deslocamento dimensional curto.','Teleporta instantaneamente até {range} unidades.','[\"mobility\", \"teleport\", \"escape\"]','[{\"type\": \"TELEPORT\", \"value_flat\": 500}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(28,'DMAGE_ARCANE_COLLAPSE','Colapso Arcano',3,13,45,1,5,1,2,8,2,0,0,0,50,30,200,0,'MANA',60,0,18000,1500,0,0,400,0,0,1,1,0,0,1,0,120,100,'Skills/DarkMage/T_Arcane_Collapse','VFX_Arcane_Burst','SFX_Collapse',NULL,NULL,NULL,300,'PHYSICAL','Colapso local do tecido mágico.','Causa {damage} de dano Arcane massivo. Alto crítico.','[\"burst\", \"magic\", \"high_crit\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 200}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(29,'DMAGE_MIST_AFFINITY','Afinidade da Névoa',3,14,48,1,5,2,1,2,4,0,0,0,45,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/DarkMage/T_Mist_Affinity',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Resistência à corrupção.','+{value} Magic Defense permanente.','[\"passive\", \"defensive\", \"magic_resist\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 40, \"target_stat\": \"magic_defense\", \"value_percent\": 12}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(30,'DMAGE_MINOR_PORTAL','Abertura do Portal Menor',3,15,50,1,5,6,4,2,2,0,0,0,55,25,250,0,'MANA',120,0,180000,2500,0,0,400,300,0,1,1,0,0,1,0,150,80,'Skills/DarkMage/T_Minor_Portal','VFX_Ultimate_DarkMage','SFX_Portal_Open',NULL,NULL,NULL,300,'PHYSICAL','Rasga o espaço por um instante.','Causa {damage} de dano Shadow massivo em área.','[\"ultimate\", \"aoe\", \"devastation\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 250}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(31,'MONK_TECH_CUT','Corte Técnico',6,1,1,1,5,1,2,1,1,0,55,0,0,0,150,20,'MANA',15,0,3000,400,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Monk/T_Tech_Cut','VFX_Precise_Slash','SFX_Swift_Cut',NULL,NULL,NULL,300,'PHYSICAL','Um golpe treinado para atingir pontos vulneráveis com eficiência fria.','Causa {damage} de dano com +{accuracy} Accuracy.','[\"melee\", \"precise\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 150}, {\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"duration_ms\": 0, \"target_stat\": \"accuracy\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(32,'MONK_HIGH_GUARD','Guarda Alta',6,2,4,1,5,3,1,1,4,0,0,50,0,0,0,0,'MANA',20,0,15000,300,8000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_High_Guard','VFX_Defense_Stance','SFX_Guard',NULL,NULL,NULL,300,'PHYSICAL','Postura defensiva clássica das escolas de guerra pré-queda.','+{value}% Physical Defense por {duration}s.','[\"buff\", \"defensive\", \"stance\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"physical_defense\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(33,'MONK_DISCIPLINE_SEQUENCE','Sequência Disciplinada',6,3,7,1,5,1,2,1,1,0,50,0,0,0,130,30,'MANA',25,0,5000,500,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Monk/T_Discipline_Sequence','VFX_Multi_Strike','SFX_Rapid_Hits',NULL,NULL,NULL,300,'PHYSICAL','Cadeia de golpes calculados sem desperdício de movimento.','Causa {damage} de dano com {double}% chance de Double Attack.','[\"melee\", \"combo\", \"double_attack\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 130}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 0, \"target_stat\": \"double_attack_rate\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(34,'MONK_TACTICAL_STEP','Passo Tático',6,4,10,1,5,3,1,1,5,0,45,0,0,0,0,0,'MANA',15,0,12000,0,4000,0,0,0,0,1,0,0,0,0,1,100,100,'Skills/Monk/T_Tactical_Step','VFX_Speed_Boost','SFX_Dash',NULL,NULL,NULL,300,'PHYSICAL','Reposicionamento preciso no caos do campo de batalha.','+{value}% Movement Speed por {duration}s.','[\"mobility\", \"buff\", \"reposition\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 4000, \"target_stat\": \"movement_speed\", \"value_percent\": 40}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(35,'MONK_PRECISE_STANCE','Postura Precisa',6,5,14,1,5,2,1,1,1,0,0,0,0,45,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_Precise_Stance',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Técnica refinada aumenta o impacto de acertos perfeitos.','+{value} Critical Attack permanente.','[\"passive\", \"offensive\", \"crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"target_stat\": \"critical_chance\", \"value_percent\": 8}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(36,'MONK_IRON_DISCIPLINE','Disciplina de Ferro',6,6,18,1,5,2,1,1,4,0,0,45,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_Iron_Discipline',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Controle mental reduz vulnerabilidade a golpes decisivos.','+{value} Critical Resistance permanente.','[\"passive\", \"defensive\", \"anti_crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"target_stat\": \"critical_resistance\", \"value_percent\": 8}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(37,'MONK_BREAKER_STRIKE','Golpe Quebrador',6,7,22,1,5,1,2,1,1,45,0,0,0,0,160,0,'MANA',30,0,8000,600,6000,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Monk/T_Breaker_Strike','VFX_Armor_Break','SFX_Armor_Crack',NULL,NULL,NULL,300,'PHYSICAL','Ataque direcionado para romper guarda.','Causa {damage} de dano e reduz Physical Defense em {debuff}% por {duration}s.','[\"melee\", \"armor_break\", \"debuff\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 160}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 6000, \"target_stat\": \"physical_defense\", \"value_percent\": -20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(38,'MONK_RIPOSTE','Ripostar',6,8,26,1,5,7,1,1,1,0,50,0,0,0,110,0,'NONE',0,0,8000,0,0,0,150,0,0,1,1,0,0,0,0,100,100,'Skills/Monk/T_Riposte','VFX_Counter_Slash','SFX_Parry',NULL,NULL,NULL,300,'PHYSICAL','Transformar defesa em punição imediata.','Ao esquivar, contra-ataca causando {damage} de dano.','[\"reaction\", \"counter\", \"trigger_on_dodge\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 110, \"conditions_json\": {\"trigger\": \"on_dodge\"}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(39,'MONK_COMBAT_BREATH','Respiração de Combate',6,9,30,1,5,1,1,1,3,0,0,55,0,0,0,0,'MANA',40,0,30000,1000,0,0,0,0,0,1,0,0,0,0,0,50,100,'Skills/Monk/T_Combat_Breath','VFX_Heal_Self','SFX_Deep_Breath',NULL,NULL,NULL,300,'PHYSICAL','Técnica respiratória usada por veteranos para se manter lutando.','Recupera {heal}% do Health máximo.','[\"heal\", \"self\", \"recovery\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(40,'MONK_BATTLE_FOCUS','Foco de Batalha',6,10,34,1,5,3,1,1,1,0,50,0,0,0,0,0,'MANA',25,0,20000,400,10000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_Battle_Focus','VFX_Focus_Aura','SFX_Focus',NULL,NULL,NULL,300,'PHYSICAL','Atenção total elimina margem de erro.','+{value} Accuracy alto por {duration}s.','[\"buff\", \"offensive\", \"accuracy\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 50, \"duration_ms\": 10000, \"target_stat\": \"accuracy\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(41,'MONK_ROTATION_DEFENSE','Defesa Rotacional',6,11,38,1,5,3,1,1,5,0,50,0,0,0,0,0,'MANA',30,0,18000,300,5000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_Rotation_Defense','VFX_Spin_Dodge','SFX_Whoosh',NULL,NULL,NULL,300,'PHYSICAL','Movimento circular que dificulta ser atingido.','+{value}% Dodge por {duration}s.','[\"buff\", \"defensive\", \"evasion\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 5000, \"target_stat\": \"dodge\", \"value_percent\": 35}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(42,'MONK_MARTIAL_EXECUTION','Execução Marcial',6,12,42,1,5,1,2,1,1,40,0,0,0,0,180,50,'MANA',45,0,14000,800,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Monk/T_Martial_Execution','VFX_Execute','SFX_Final_Blow',NULL,NULL,NULL,300,'PHYSICAL','Finalização técnica sem hesitação.','Causa {damage} de dano. +{bonus}% dano se alvo abaixo de 30% HP.','[\"melee\", \"execute\", \"finisher\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 180}, {\"type\": \"EXECUTE\", \"value_percent\": 50, \"conditions_json\": {\"target_health_below_percent\": 30}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(43,'MONK_WAR_RHYTHM','Ritmo de Guerra',6,13,45,1,5,3,1,1,1,0,55,0,0,0,0,0,'MANA',40,0,30000,500,15000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_War_Rhythm','VFX_Combat_Flow','SFX_Rhythm',NULL,NULL,NULL,300,'PHYSICAL','Entra no fluxo perfeito de combate.','+{double}% Double Attack, +{speed}% Speed por {duration}s.','[\"buff\", \"offensive\", \"combo\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"double_attack_rate\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"movement_speed\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(44,'MONK_MENTAL_ARMOR','Armadura Mental',6,14,48,1,5,2,1,1,4,0,0,30,30,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Monk/T_Mental_Armor',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Treino contra terror e magia hostil.','+{value} Magic Defense permanente.','[\"passive\", \"defensive\", \"magic_resist\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 35, \"target_stat\": \"magic_defense\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(45,'MONK_VETERAN_FORM','Forma do Veterano',6,15,50,1,5,6,1,1,1,40,40,30,0,0,0,0,'MANA',100,0,180000,1500,20000,0,0,0,0,1,0,0,0,0,0,150,80,'Skills/Monk/T_Veteran_Form','VFX_Ultimate_Monk','SFX_Ultimate_Focus',NULL,NULL,NULL,300,'PHYSICAL','Aplica todas as doutrinas de guerra de uma vez.','Por {duration}s: +{atk}% Phys Atk, +{def}% Defense, +{acc} Accuracy, +{dodge} Dodge.','[\"ultimate\", \"buff\", \"balanced\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"physical_attack\", \"value_percent\": 30}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"physical_defense\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"value_flat\": 40, \"duration_ms\": 20000, \"target_stat\": \"accuracy\"}, {\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"duration_ms\": 20000, \"target_stat\": \"dodge\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(46,'TEMP_HOLY_STRIKE','Golpe Consagrado',2,1,1,1,5,1,2,4,1,35,0,0,35,0,145,0,'MANA',18,0,4000,500,0,0,150,0,0,1,1,0,0,1,0,120,100,'Skills/Templar/T_Holy_Strike','VFX_Holy_Slash','SFX_Holy_Impact',NULL,NULL,NULL,300,'PHYSICAL','Ataque carregado com runas das Cinzas.','Causa {damage} de dano híbrido (Físico + Holy).','[\"melee\", \"hybrid\", \"holy\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 145}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(47,'TEMP_VOW_SHIELD','Escudo de Voto',2,2,4,1,5,3,1,4,4,0,0,55,0,0,0,0,'MANA',25,0,18000,400,10000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Vow_Shield','VFX_Holy_Shield','SFX_Shield_Up',NULL,NULL,NULL,300,'PHYSICAL','Escudo energizado por juramento ritual.','+{value}% Physical Defense por {duration}s.','[\"buff\", \"defensive\", \"shield\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"physical_defense\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(48,'TEMP_SPIRIT_AEGIS','Égide Espiritual',2,3,7,1,5,3,1,4,4,0,0,0,50,0,0,0,'MANA',25,0,18000,400,10000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Spirit_Aegis','VFX_Magic_Barrier','SFX_Barrier',NULL,NULL,NULL,300,'PHYSICAL','Proteção contra corrupção dimensional.','+{value}% Magic Defense por {duration}s.','[\"buff\", \"defensive\", \"magic_resist\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"magic_defense\", \"value_percent\": 30}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(49,'TEMP_ASH_LIGHT','Luz das Cinzas',2,4,10,1,5,1,2,4,2,0,0,0,55,0,160,30,'MANA',30,0,6000,800,0,0,300,0,0,1,1,0,0,1,0,100,100,'Skills/Templar/T_Ash_Light','VFX_Holy_Beam','SFX_Holy_Blast',NULL,NULL,NULL,300,'PHYSICAL','Energia purificadora rara.','Causa {damage} de dano Holy. +{bonus}% dano contra criaturas da Neblina.','[\"ranged\", \"holy\", \"anti_shadow\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 160}, {\"type\": \"DAMAGE\", \"value_percent\": 30, \"conditions_json\": {\"target_type\": \"shadow_creature\"}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(50,'TEMP_GUARDIAN_AURA','Aura do Guardião',2,5,14,1,5,5,5,4,4,0,0,50,0,0,0,0,'MANA',0,5,60000,1000,0,0,0,300,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Guardian_Aura','VFX_Aura_Protection','SFX_Aura_Hum',NULL,NULL,NULL,300,'PHYSICAL','Presença protetora constante.','+{value} Critical Resistance para aliados na área.','[\"aura\", \"party\", \"defensive\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"target_stat\": \"critical_resistance\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(51,'TEMP_GUARDIAN_STEP','Passo Guardião',2,6,18,1,5,3,1,4,4,0,0,55,0,0,0,0,'MANA',30,0,15000,300,6000,0,0,0,0,1,0,0,0,0,1,150,100,'Skills/Templar/T_Guardian_Step','VFX_Holy_Charge','SFX_Heavy_Step',NULL,NULL,NULL,300,'PHYSICAL','Avança absorvendo impacto.','-{value}% dano recebido por {duration}s.','[\"buff\", \"defensive\", \"damage_reduction\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 6000, \"target_stat\": \"damage_reduction\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(52,'TEMP_JUDGMENT','Julgamento',2,7,22,1,5,1,2,4,1,0,0,0,0,50,170,0,'MANA',35,0,10000,700,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Templar/T_Judgment','VFX_Holy_Judgment','SFX_Condemn',NULL,NULL,NULL,300,'PHYSICAL','Golpe de condenação ritual.','Causa {damage} de dano Holy com alto crítico.','[\"melee\", \"holy\", \"high_crit\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 170}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(53,'TEMP_RUNIC_BARRIER','Barreira Rúnica',2,8,26,1,5,3,1,4,3,0,0,0,50,0,0,0,'MANA',40,0,25000,800,10000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Runic_Barrier','VFX_Rune_Shield','SFX_Rune_Activate',NULL,NULL,NULL,300,'PHYSICAL','Selo defensivo projetado.','Cria escudo que absorve {value} de dano por {duration}s.','[\"shield\", \"absorb\", \"defensive\"]','[{\"type\": \"SHIELD\", \"value_flat\": 300, \"duration_ms\": 10000, \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(54,'TEMP_FIELD_PRAYER','Oração de Campo',2,9,30,1,5,1,3,4,3,0,0,40,40,0,0,0,'MANA',45,0,12000,1200,0,0,250,0,0,1,0,0,0,1,0,150,100,'Skills/Templar/T_Field_Prayer','VFX_Heal_Single','SFX_Prayer',NULL,NULL,NULL,300,'PHYSICAL','Prece curta de restauração.','Cura {heal}% do Health máximo do aliado.','[\"heal\", \"single_target\", \"support\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(55,'TEMP_VOW_MARK','Marca do Juramento',2,10,34,1,5,4,2,4,2,0,0,0,50,0,0,0,'MANA',30,0,15000,600,10000,0,300,0,0,1,0,0,0,1,0,200,100,'Skills/Templar/T_Vow_Mark','VFX_Holy_Mark','SFX_Mark',NULL,NULL,NULL,300,'PHYSICAL','Marca quem deve ser contido.','Reduz Attack do alvo em {value}% por {duration}s.','[\"debuff\", \"control\", \"weaken\"]','[{\"type\": \"DEBUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"physical_attack\", \"value_percent\": -20}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"magic_attack\", \"value_percent\": -20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(56,'TEMP_DEVOUT_RESISTANCE','Resistência Devota',2,11,38,1,5,2,1,4,3,0,0,55,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Devout_Resistance',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Corpo fortalecido pela fé marcial.','+{value} Health Máximo permanente.','[\"passive\", \"defensive\", \"health\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 250, \"target_stat\": \"max_health\", \"value_percent\": 15}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(57,'TEMP_INTERPOSITION','Interposição',2,12,42,1,5,7,3,4,4,0,0,60,0,0,0,0,'NONE',0,0,30000,0,0,0,200,0,0,1,0,0,0,1,0,300,100,'Skills/Templar/T_Interposition','VFX_Protect_Ally','SFX_Block',NULL,NULL,NULL,300,'PHYSICAL','Proteção sacrificial.','Redireciona {value}% do dano de um aliado para si.','[\"reaction\", \"tank\", \"redirect\"]','[{\"type\": \"REFLECT\", \"value_percent\": 50, \"conditions_json\": {\"trigger\": \"ally_damaged\", \"redirect_to_self\": true}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(58,'TEMP_VOW_FLAME','Chama do Voto',2,13,45,1,5,3,5,4,1,45,0,0,0,0,0,0,'MANA',50,0,35000,1000,15000,0,0,400,0,1,0,0,0,0,0,120,100,'Skills/Templar/T_Vow_Flame','VFX_Holy_Empower','SFX_Fire_Holy',NULL,NULL,NULL,300,'PHYSICAL','Convoca fervor de guerra.','+{value}% Physical Attack para aliados por {duration}s.','[\"buff\", \"party\", \"offensive\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"physical_attack\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(59,'TEMP_UNBREAKABLE_SHIELD','Escudo Inquebrável',2,14,48,1,5,2,1,4,4,0,0,50,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Templar/T_Unbreakable_Shield',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Defesa contra cadeias de golpes.','+{value} Double Attack Resistance permanente.','[\"passive\", \"defensive\", \"anti_double\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"target_stat\": \"double_attack_resistance\", \"value_percent\": 12}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(60,'TEMP_ASH_MIRACLE','Milagre das Cinzas',2,15,50,1,5,6,6,4,3,0,0,45,40,0,0,0,'MANA',120,0,180000,2500,0,0,0,400,0,1,0,0,0,0,0,200,80,'Skills/Templar/T_Ash_Miracle','VFX_Ultimate_Templar','SFX_Miracle',NULL,NULL,NULL,300,'PHYSICAL','Ritual maior templário.','Cura {heal}% do Health de aliados em área e concede +{def}% Defense por {duration}s.','[\"ultimate\", \"heal\", \"aoe_ally\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 35}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"physical_defense\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 15000, \"target_stat\": \"magic_defense\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(61,'ALCH_FIELD_TONIC','Tônico de Campo',4,1,1,1,5,1,3,5,3,0,0,0,50,0,0,0,'MANA',20,0,6000,600,0,0,250,0,0,1,0,0,0,1,0,150,100,'Skills/Alchemist/T_Field_Tonic','VFX_Heal_Potion','SFX_Potion',NULL,NULL,NULL,300,'PHYSICAL','Tratamento rápido com reagentes instáveis.','Cura {heal}% do Health máximo do aliado.','[\"heal\", \"single_target\", \"support\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(62,'ALCH_STRENGTH_ELIXIR','Elixir de Força',4,2,4,1,5,3,3,5,5,50,0,0,0,0,0,0,'MANA',25,0,20000,500,60000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Strength_Elixir','VFX_Buff_Red','SFX_Drink',NULL,NULL,NULL,300,'PHYSICAL','Mistura que estimula fibras musculares.','+{value} Strength para aliado por {duration}s.','[\"buff\", \"ally\", \"strength\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"duration_ms\": 60000, \"target_stat\": \"strength\", \"value_percent\": 15}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(63,'ALCH_PRECISION_SERUM','Soro de Precisão',4,3,7,1,5,3,3,5,5,0,50,0,0,0,0,0,'MANA',25,0,20000,500,60000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Precision_Serum','VFX_Buff_Green','SFX_Drink',NULL,NULL,NULL,300,'PHYSICAL','Ajusta reflexos e foco.','+{value} Dexterity e Accuracy para aliado por {duration}s.','[\"buff\", \"ally\", \"dexterity\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"duration_ms\": 60000, \"target_stat\": \"dexterity\", \"value_percent\": 12}, {\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"duration_ms\": 60000, \"target_stat\": \"accuracy\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(64,'ALCH_VIGOR_COMPOUND','Composto Revigorante',4,4,10,1,5,3,3,5,5,0,0,50,0,0,0,0,'MANA',25,0,20000,500,60000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Vigor_Compound','VFX_Buff_Orange','SFX_Drink',NULL,NULL,NULL,300,'PHYSICAL','Endurece resistência corporal.','+{value} Vitality e Max Health para aliado por {duration}s.','[\"buff\", \"ally\", \"vitality\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"duration_ms\": 60000, \"target_stat\": \"vitality\", \"value_percent\": 12}, {\"type\": \"BUFF_STAT\", \"value_flat\": 100, \"duration_ms\": 60000, \"target_stat\": \"max_health\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(65,'ALCH_COGNITIVE_INFUSION','Infusão Cognitiva',4,5,14,1,5,3,3,8,5,0,0,0,50,0,0,0,'MANA',25,0,20000,500,60000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Cognitive_Infusion','VFX_Buff_Blue','SFX_Drink',NULL,NULL,NULL,300,'PHYSICAL','Clareza mental alquímica.','+{value} Intelligence e Magic Attack para aliado por {duration}s.','[\"buff\", \"ally\", \"intelligence\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"duration_ms\": 60000, \"target_stat\": \"intelligence\", \"value_percent\": 12}, {\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"duration_ms\": 60000, \"target_stat\": \"magic_attack\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(66,'ALCH_FORTUNE_DISTILLATE','Destilado da Fortuna',4,6,18,1,5,3,3,8,5,0,0,0,0,50,0,0,'MANA',30,0,25000,500,60000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Fortune_Distillate','VFX_Buff_Gold','SFX_Drink',NULL,NULL,NULL,300,'PHYSICAL','Catalisador probabilístico raro.','+{value} Luck e Critical Chance para aliado por {duration}s.','[\"buff\", \"ally\", \"luck\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"duration_ms\": 60000, \"target_stat\": \"luck\", \"value_percent\": 15}, {\"type\": \"BUFF_STAT\", \"value_flat\": 15, \"duration_ms\": 60000, \"target_stat\": \"critical_chance\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(67,'ALCH_RESTORING_MIST','Névoa Restauradora',4,7,22,1,5,1,6,5,3,0,0,0,55,0,0,0,'MANA',50,0,18000,1000,0,0,0,300,0,1,0,0,0,0,0,180,100,'Skills/Alchemist/T_Restoring_Mist','VFX_Heal_AoE','SFX_Mist',NULL,NULL,NULL,300,'PHYSICAL','Vapores curativos.','Cura {heal}% do Health de aliados em área.','[\"heal\", \"aoe_ally\", \"support\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 18}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(68,'ALCH_PROTECTIVE_COAT','Revestimento Protetor',4,8,26,1,5,3,3,5,4,0,0,50,0,0,0,0,'MANA',35,0,22000,600,30000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Protective_Coat','VFX_Buff_Shield','SFX_Apply',NULL,NULL,NULL,300,'PHYSICAL','Camada química protetora.','+{phys}% Physical Defense e +{mag}% Magic Defense para aliado por {duration}s.','[\"buff\", \"ally\", \"defensive\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 30000, \"target_stat\": \"physical_defense\", \"value_percent\": 20}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 30000, \"target_stat\": \"magic_defense\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(69,'ALCH_COMBAT_STIMULANT','Estimulante de Combate',4,9,30,1,5,3,3,5,5,0,45,0,0,0,0,0,'MANA',35,0,25000,500,20000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Combat_Stimulant','VFX_Speed_Buff','SFX_Inject',NULL,NULL,NULL,300,'PHYSICAL','Acelera respostas neurais.','+{speed}% Movement Speed e +{double}% Double Attack para aliado por {duration}s.','[\"buff\", \"ally\", \"offensive\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"movement_speed\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"double_attack_rate\", \"value_percent\": 20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(70,'ALCH_NEUTRALIZER','Neutralizador',4,10,34,1,5,1,3,5,5,0,0,0,50,0,0,0,'MANA',40,0,15000,300,0,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Neutralizer','VFX_Cleanse','SFX_Cleanse',NULL,NULL,NULL,300,'PHYSICAL','Antídoto universal parcial.','Remove até {value} debuffs do aliado.','[\"cleanse\", \"support\", \"dispel\"]','[{\"type\": \"CLEANSE\", \"value_flat\": 3}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(71,'ALCH_STABILIZATION_VIAL','Ampola de Estabilização',4,11,38,1,5,3,3,5,3,0,0,50,0,0,0,0,'MANA',45,0,20000,500,8000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Stabilization_Vial','VFX_Shield_Ally','SFX_Shield',NULL,NULL,NULL,300,'PHYSICAL','Gel reativo absorvedor.','Cria escudo que absorve {value} de dano no aliado por {duration}s.','[\"shield\", \"ally\", \"absorb\"]','[{\"type\": \"SHIELD\", \"value_flat\": 350, \"duration_ms\": 8000, \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(72,'ALCH_MANA_CATALYST','Catalisador de Mana',4,12,42,1,5,1,3,8,5,0,0,0,50,0,0,0,'MANA',30,0,30000,600,10000,0,250,0,0,1,0,0,0,1,0,100,100,'Skills/Alchemist/T_Mana_Catalyst','VFX_Mana_Restore','SFX_Magic',NULL,NULL,NULL,300,'PHYSICAL','Recarrega fluxo arcano.','Regenera {value}% do Mana máximo do aliado ao longo de {duration}s.','[\"resource\", \"ally\", \"mana\"]','[{\"type\": \"HOT\", \"duration_ms\": 10000, \"target_stat\": \"mana\", \"value_percent\": 30, \"tick_interval_ms\": 2000}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(73,'ALCH_DISRUPTIVE_BOMB','Bomba Disruptiva',4,13,45,1,5,4,4,5,2,0,0,0,0,50,60,0,'MANA',45,0,20000,800,8000,0,300,200,0,1,0,0,0,1,0,80,100,'Skills/Alchemist/T_Disruptive_Bomb','VFX_Smoke_Bomb','SFX_Explosion',NULL,NULL,NULL,300,'PHYSICAL','Explosão química cegante.','Causa {damage} de dano e reduz Accuracy e Critical em {debuff}% dos inimigos por {duration}s.','[\"debuff\", \"aoe\", \"control\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 60}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"accuracy\", \"value_percent\": -25}, {\"type\": \"DEBUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"critical_chance\", \"value_percent\": -20}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(74,'ALCH_ADVANCED_PREP','Preparação Antecipada',4,14,48,1,5,2,1,5,5,0,0,0,50,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Alchemist/T_Advanced_Prep',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Planejamento alquímico.','Buffs aplicados duram {value}% mais tempo.','[\"passive\", \"support\", \"buff_duration\"]','[{\"type\": \"BUFF_STAT\", \"target_stat\": \"buff_duration\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(75,'ALCH_SURVIVAL_PROTOCOL','Protocolo de Sobrevivência',4,15,50,1,5,6,5,5,3,0,0,40,45,25,0,0,'MANA',130,0,180000,2000,0,0,0,500,0,1,0,0,0,0,0,250,80,'Skills/Alchemist/T_Survival_Protocol','VFX_Ultimate_Alchemist','SFX_Ultimate_Support',NULL,NULL,NULL,300,'PHYSICAL','Sequência completa de suporte.','Cura {heal}% do Health de aliados e concede +{def}% Defense, +{res} Resistances por {duration}s.','[\"ultimate\", \"party\", \"full_support\"]','[{\"type\": \"HEAL\", \"target_stat\": \"health\", \"value_percent\": 30}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"physical_defense\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 20000, \"target_stat\": \"magic_defense\", \"value_percent\": 25}, {\"type\": \"BUFF_STAT\", \"value_flat\": 20, \"duration_ms\": 20000, \"target_stat\": \"all_resistance\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(76,'ASSN_SHADOW_STRIKE','Golpe nas Sombras',5,1,1,1,5,1,2,2,1,0,0,0,0,55,160,0,'MANA',15,0,4000,300,0,0,150,0,0,1,1,0,0,1,0,80,100,'Skills/Assassin/T_Shadow_Strike','VFX_Stealth_Hit','SFX_Swift_Stab',NULL,NULL,NULL,300,'PHYSICAL','Ataque invisível inicial.','Causa {damage} de dano com alto crítico.','[\"melee\", \"crit\", \"opener\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 160}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(77,'ASSN_INVISIBLE_STEP','Passo Invisível',5,2,4,1,5,3,1,2,5,0,50,0,0,0,0,0,'MANA',20,0,15000,0,5000,0,0,0,0,1,0,0,0,0,1,100,100,'Skills/Assassin/T_Invisible_Step','VFX_Stealth','SFX_Vanish',NULL,NULL,NULL,300,'PHYSICAL','Movimento fora da linha de visão.','+{value}% Dodge por {duration}s.','[\"buff\", \"evasion\", \"stealth\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 5000, \"target_stat\": \"dodge\", \"value_percent\": 40}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(78,'ASSN_PRECISE_BLADE','Lâmina Precisa',5,3,7,1,5,1,2,1,1,0,55,0,0,0,140,30,'MANA',18,0,3500,250,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Assassin/T_Precise_Blade','VFX_Quick_Slash','SFX_Sharp_Cut',NULL,NULL,NULL,300,'PHYSICAL','Ajuste fino de golpe.','Causa {damage} de dano com +{accuracy} Accuracy.','[\"melee\", \"precise\", \"accurate\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 140}, {\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"duration_ms\": 0, \"target_stat\": \"accuracy\"}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(79,'ASSN_DOUBLE_ATTACK','Ataque Duplo',5,4,10,1,5,1,2,1,1,0,55,0,0,0,110,0,'MANA',22,0,5000,300,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Assassin/T_Double_Attack','VFX_Dual_Strike','SFX_Double_Hit',NULL,NULL,NULL,300,'PHYSICAL','Sequência rápida.','Ataca duas vezes causando {damage} de dano cada.','[\"melee\", \"multi_hit\", \"combo\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 110}, {\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 110}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(80,'ASSN_TENDON_CUT','Corte de Tendão',5,5,14,1,5,1,2,1,1,0,50,0,0,0,120,0,'MANA',25,0,10000,400,6000,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Assassin/T_Tendon_Cut','VFX_Cripple','SFX_Slice',NULL,NULL,NULL,300,'PHYSICAL','Imobiliza a presa.','Causa {damage} de dano e reduz Movement Speed em {slow}% por {duration}s.','[\"melee\", \"slow\", \"control\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 120}, {\"type\": \"SLOW\", \"duration_ms\": 6000, \"value_percent\": -40}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(81,'ASSN_DEEP_BLEED','Sangramento Profundo',5,6,18,1,5,8,2,1,1,0,0,0,0,50,40,70,'MANA',28,0,12000,500,10000,0,150,0,1,3,0,0,0,1,0,100,100,'Skills/Assassin/T_Deep_Bleed','VFX_Bleed_Heavy','SFX_Deep_Cut',NULL,NULL,NULL,300,'PHYSICAL','Ferida que não fecha.','Aplica sangramento: {dot} dano a cada {tick}s por {duration}s.','[\"dot\", \"bleed\", \"sustained\"]','[{\"type\": \"DOT\", \"duration_ms\": 10000, \"target_stat\": \"health\", \"value_percent\": 70, \"tick_interval_ms\": 2000}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(82,'ASSN_DEADLY_REFLEXES','Reflexos Mortais',5,7,22,1,5,7,1,2,5,0,55,0,0,0,0,0,'NONE',0,0,15000,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Assassin/T_Deadly_Reflexes','VFX_Dodge_Flash','SFX_Whoosh',NULL,NULL,NULL,300,'PHYSICAL','Desvio instantâneo.','Esquiva automaticamente do próximo ataque.','[\"reaction\", \"evasion\", \"survival\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 500, \"target_stat\": \"dodge\", \"value_percent\": 100, \"conditions_json\": {\"trigger\": \"on_attack_received\", \"max_uses\": 1}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(83,'ASSN_NIGHT_HUNTER','Caçador Noturno',5,8,26,1,5,2,1,2,1,0,0,0,0,50,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Assassin/T_Night_Hunter',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Especialista em morte rápida.','+{value} Critical Attack permanente.','[\"passive\", \"offensive\", \"crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 25, \"target_stat\": \"critical_chance\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(84,'ASSN_SHADOW_RECOVERY','Recuperação Sombria',5,9,30,1,5,2,1,2,3,0,0,40,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Assassin/T_Shadow_Recovery',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Roubo de vitalidade.','Ataques curam {value}% do dano causado.','[\"passive\", \"lifesteal\", \"sustain\"]','[{\"type\": \"LIFESTEAL\", \"value_percent\": 8}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(85,'ASSN_SMOKE_VEIL','Véu de Fumaça',5,10,34,1,5,3,1,2,5,0,55,0,0,0,0,0,'MANA',30,0,25000,0,4000,0,0,0,0,1,0,0,0,0,1,100,100,'Skills/Assassin/T_Smoke_Veil','VFX_Smoke_Cloud','SFX_Smoke',NULL,NULL,NULL,300,'PHYSICAL','Desaparecimento tático.','+{value}% Dodge muito alto por {duration}s.','[\"buff\", \"evasion\", \"escape\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 4000, \"target_stat\": \"dodge\", \"value_percent\": 60}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(86,'ASSN_EXECUTION','Execução',5,11,38,1,5,1,2,2,1,0,0,0,0,55,200,80,'MANA',50,0,20000,600,0,0,150,0,0,1,1,0,0,1,0,100,100,'Skills/Assassin/T_Execution','VFX_Execute_Shadow','SFX_Execute',NULL,NULL,NULL,300,'PHYSICAL','Finalização precisa.','Causa {damage} de dano. +{bonus}% dano se alvo abaixo de 25% HP.','[\"melee\", \"execute\", \"finisher\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 200}, {\"type\": \"EXECUTE\", \"value_percent\": 80, \"conditions_json\": {\"target_health_below_percent\": 25}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(87,'ASSN_PREDATOR_EYE','Olho do Predador',5,12,42,1,5,3,1,2,1,0,50,0,0,0,0,0,'MANA',35,0,30000,300,10000,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Assassin/T_Predator_Eye','VFX_Eye_Glow','SFX_Focus',NULL,NULL,NULL,300,'PHYSICAL','Nada escapa.','Ataques ignoram {value}% do Dodge inimigo por {duration}s.','[\"buff\", \"offensive\", \"anti_dodge\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 10000, \"target_stat\": \"ignore_dodge\", \"value_percent\": 50}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(88,'ASSN_STEP_BETWEEN','Passo Entre Golpes',5,13,45,1,5,3,1,2,5,0,55,0,0,0,0,0,'MANA',35,0,25000,0,8000,0,0,0,0,1,0,0,0,0,1,100,100,'Skills/Assassin/T_Step_Between','VFX_Phase_Move','SFX_Blink',NULL,NULL,NULL,300,'PHYSICAL','Reposicionamento letal.','+{speed}% Movement Speed e +{double}% Double Attack por {duration}s.','[\"buff\", \"mobility\", \"offensive\"]','[{\"type\": \"BUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"movement_speed\", \"value_percent\": 35}, {\"type\": \"BUFF_STAT\", \"duration_ms\": 8000, \"target_stat\": \"double_attack_rate\", \"value_percent\": 25}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(89,'ASSN_COLD_NERVES','Nervos Frios',5,14,48,1,5,2,1,2,4,0,0,45,0,0,0,0,'NONE',0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,100,100,'Skills/Assassin/T_Cold_Nerves',NULL,NULL,NULL,NULL,NULL,300,'PHYSICAL','Controle emocional absoluto.','+{value} Critical Resistance permanente.','[\"passive\", \"defensive\", \"anti_crit\"]','[{\"type\": \"BUFF_STAT\", \"value_flat\": 30, \"target_stat\": \"critical_resistance\", \"value_percent\": 10}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(90,'ASSN_PERFECT_KILL','Assassinato Perfeito',5,15,50,1,5,6,2,2,1,0,50,0,0,40,280,0,'MANA',100,0,180000,500,0,0,150,0,0,1,1,0,0,1,0,120,80,'Skills/Assassin/T_Perfect_Kill','VFX_Ultimate_Assassin','SFX_Ultimate_Strike',NULL,NULL,NULL,300,'PHYSICAL','Execução sem erro.','Sequência de ataques garantindo crítico. Causa {damage} de dano com 100% Critical.','[\"ultimate\", \"burst\", \"guaranteed_crit\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 280, \"conditions_json\": {\"guaranteed_crit\": true}}]',1,0,'2026-02-21 21:33:52','2026-02-21 21:33:52'),(91,'BARB_BASIC_ATTACK','Ataque Básico',1,0,1,0,1,1,2,1,1,40,0,0,0,0,85,0,'NONE',0,0,800,0,0,0,250,0,0,1,1,0,0,1,0,100,100,'Skills/Barbarian/T_Basic_Attack','VFX_BasicHit_Physical','SFX_BasicHit_Physical','/Game/Animations/Combat/AM_Barb_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Physical','/Game/SFX/Combat/SFX_BasicHit_Physical',300,'PHYSICAL','Ataque básico corpo a corpo da Barbarian. Sempre disponível.','Causa {damage} de dano físico.','[\"melee\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 85}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39'),(92,'TEMP_BASIC_ATTACK','Ataque Básico',2,0,1,0,1,1,2,4,1,35,0,0,35,0,82,0,'NONE',0,0,850,0,0,0,250,0,0,1,1,0,0,1,0,100,100,'Skills/Templar/T_Basic_Attack','VFX_BasicHit_Holy','SFX_BasicHit_Holy','/Game/Animations/Combat/AM_Temp_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Holy','/Game/SFX/Combat/SFX_BasicHit_Holy',320,'PHYSICAL','Ataque básico consagrado do Templar. Sempre disponível.','Causa {damage} de dano físico sagrado.','[\"melee\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 82}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39'),(93,'DMAGE_BASIC_ATTACK','Ataque Básico',3,0,1,0,1,1,2,2,2,0,0,0,50,0,78,0,'NONE',0,0,900,0,0,0,300,0,0,1,1,0,0,1,0,100,100,'Skills/DarkMage/T_Basic_Attack','VFX_BasicHit_Shadow','SFX_BasicHit_Shadow','/Game/Animations/Combat/AM_DMage_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Shadow','/Game/SFX/Combat/SFX_BasicHit_Shadow',350,'MAGIC','Ataque básico sombrio da Dark Mage. Sempre disponível.','Causa {damage} de dano mágico.','[\"ranged\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 78}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39'),(94,'CLERIC_BASIC_ATTACK','Ataque Básico',4,0,1,0,1,1,2,4,2,0,0,0,45,0,80,0,'NONE',0,0,900,0,0,0,280,0,0,1,1,0,0,1,0,100,100,'Skills/Cleric/T_Basic_Attack','VFX_BasicHit_Holy','SFX_BasicHit_Holy','/Game/Animations/Combat/AM_Cleric_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Holy','/Game/SFX/Combat/SFX_BasicHit_Holy',340,'MAGIC','Ataque básico do Cleric. Sempre disponível.','Causa {damage} de dano mágico.','[\"ranged\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 80}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39'),(95,'ASSN_BASIC_ATTACK','Ataque Básico',5,0,1,0,1,1,2,1,1,30,40,0,0,0,88,0,'NONE',0,0,700,0,0,0,220,0,0,1,1,0,0,1,0,100,100,'Skills/Assassin/T_Basic_Attack','VFX_BasicHit_Crit','SFX_BasicHit_Crit','/Game/Animations/Combat/AM_Assn_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Crit','/Game/SFX/Combat/SFX_BasicHit_Crit',250,'PHYSICAL','Ataque básico rápido do Assassin. Sempre disponível.','Causa {damage} de dano físico.','[\"melee\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 88}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39'),(96,'MONK_BASIC_ATTACK','Ataque Básico',6,0,1,0,1,1,2,1,1,35,25,0,0,0,84,0,'NONE',0,0,750,0,0,0,240,0,0,1,1,0,0,1,0,100,100,'Skills/Monk/T_Basic_Attack','VFX_BasicHit_Wind','SFX_BasicHit_Wind','/Game/Animations/Combat/AM_Monk_BasicAttack','/Game/VFX/Combat/VFX_BasicHit_Wind','/Game/SFX/Combat/SFX_BasicHit_Wind',280,'PHYSICAL','Ataque básico marcial do Monk. Sempre disponível.','Causa {damage} de dano físico.','[\"melee\", \"basic_attack\", \"single_target\"]','[{\"type\": \"DAMAGE\", \"target_stat\": \"health\", \"value_percent\": 84}]',1,1,'2026-06-20 01:34:39','2026-06-20 01:34:39');
/*!40000 ALTER TABLE `skills` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `spawn_points`
--

DROP TABLE IF EXISTS `spawn_points`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `spawn_points` (
  `spawn_id` int unsigned NOT NULL AUTO_INCREMENT,
  `spawn_key` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `zone_id` int unsigned NOT NULL DEFAULT '1',
  `pos_x` float NOT NULL,
  `pos_y` float NOT NULL,
  `pos_z` float NOT NULL,
  `yaw` float NOT NULL DEFAULT '0',
  `is_default` tinyint(1) NOT NULL DEFAULT '0',
  `display_name` varchar(128) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`spawn_id`),
  UNIQUE KEY `uk_zone_key` (`zone_id`,`spawn_key`),
  KEY `idx_zone_default` (`zone_id`,`is_default`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `spawn_points`
--

LOCK TABLES `spawn_points` WRITE;
/*!40000 ALTER TABLE `spawn_points` DISABLE KEYS */;
INSERT INTO `spawn_points` VALUES (1,'Lvl_Tutorial_',1,-2580,-980,140,0,1,'Cidade Inicial');
/*!40000 ALTER TABLE `spawn_points` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `titles`
--

DROP TABLE IF EXISTS `titles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `titles` (
  `title_id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `title_name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `title_description` text COLLATE utf8mb4_unicode_ci,
  `title_status` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT 'common',
  `bonus_strength` int DEFAULT '0',
  `bonus_dexterity` int DEFAULT '0',
  `bonus_intelligence` int DEFAULT '0',
  `bonus_vitality` int DEFAULT '0',
  `bonus_luck` int DEFAULT '0',
  `bonus_health` int DEFAULT '0',
  `bonus_mana` int DEFAULT '0',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`title_id`),
  UNIQUE KEY `title_name` (`title_name`),
  KEY `idx_title_name` (`title_name`),
  KEY `idx_title_status` (`title_status`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `titles`
--

LOCK TABLES `titles` WRITE;
/*!40000 ALTER TABLE `titles` DISABLE KEYS */;
INSERT INTO `titles` VALUES (1,'Novato','Um iniciante em sua jornada. Este título não oferece bônus, mas marca o início de uma grande aventura.','common',0,0,0,0,0,0,0,'2025-11-27 16:19:42'),(2,'Veterano','Um guerreiro experiente que já enfrentou muitos desafios. +2 em todos os atributos base.','uncommon',2,2,2,2,2,20,10,'2025-11-27 16:19:42'),(3,'Lendário','Um herói cujas façanhas são conhecidas em todo o reino. +5 em todos os atributos base e +50 de vida.','rare',5,5,5,5,5,50,25,'2025-11-27 16:19:42'),(4,'Mestre das Sombras','Um especialista em combate furtivo. +8 em Destreza, +5 em Sorte e +10% de chance de crítico.','epic',0,8,0,0,5,0,0,'2025-11-27 16:19:42'),(5,'Guardião Eterno','O protetor supremo, cuja força é lendária. +10 em Força, +8 em Vitalidade e +100 de vida máxima.','legendary',10,0,0,8,0,100,0,'2025-11-27 16:19:42');
/*!40000 ALTER TABLE `titles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `trade_items`
--

DROP TABLE IF EXISTS `trade_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `trade_items` (
  `trade_item_id` int unsigned NOT NULL AUTO_INCREMENT,
  `trade_session_id` int unsigned NOT NULL,
  `player_id` bigint unsigned NOT NULL COMMENT 'Jogador que está oferecendo',
  `inventory_id` int NOT NULL COMMENT 'Item do inventário',
  `quantity` int unsigned NOT NULL DEFAULT '1',
  `added_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`trade_item_id`),
  KEY `idx_trade_session` (`trade_session_id`),
  KEY `idx_player` (`player_id`),
  KEY `inventory_id` (`inventory_id`),
  CONSTRAINT `trade_items_ibfk_1` FOREIGN KEY (`trade_session_id`) REFERENCES `trade_sessions` (`trade_session_id`) ON DELETE CASCADE,
  CONSTRAINT `trade_items_ibfk_2` FOREIGN KEY (`inventory_id`) REFERENCES `player_inventory` (`inventory_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=123 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `trade_items`
--

LOCK TABLES `trade_items` WRITE;
/*!40000 ALTER TABLE `trade_items` DISABLE KEYS */;
INSERT INTO `trade_items` VALUES (6,17,1,100,1,'2026-02-07 23:51:29'),(10,18,1,100,1,'2026-02-07 23:56:56'),(16,20,1,65,1,'2026-02-08 00:03:43'),(20,22,1,91,1,'2026-02-08 01:22:40'),(21,22,1,65,1,'2026-02-08 01:22:51'),(28,25,1,91,1,'2026-02-08 02:46:08'),(32,27,1,91,1,'2026-02-08 03:10:50'),(33,28,1,91,1,'2026-02-08 03:14:40'),(36,29,1,91,1,'2026-02-08 14:23:53'),(38,30,1,91,1,'2026-02-08 14:27:24'),(42,36,23,91,1,'2026-02-08 15:20:54'),(45,47,1,95,1,'2026-02-08 18:56:30');
/*!40000 ALTER TABLE `trade_items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `trade_requests`
--

DROP TABLE IF EXISTS `trade_requests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `trade_requests` (
  `request_id` int unsigned NOT NULL AUTO_INCREMENT,
  `trade_session_id` int unsigned DEFAULT NULL COMMENT 'ID da sessão de troca (NULL = pendente)',
  `from_player_id` bigint unsigned NOT NULL,
  `to_player_id` bigint unsigned NOT NULL,
  `status` enum('pending','accepted','declined','expired','cancelled') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `responded_at` timestamp NULL DEFAULT NULL,
  `expires_at` timestamp NULL DEFAULT NULL COMMENT 'Solicitação expira após 2 minutos',
  PRIMARY KEY (`request_id`),
  KEY `idx_from_player` (`from_player_id`),
  KEY `idx_to_player` (`to_player_id`),
  KEY `idx_trade_session` (`trade_session_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `trade_requests_ibfk_1` FOREIGN KEY (`from_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `trade_requests_ibfk_2` FOREIGN KEY (`to_player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=217 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `trade_requests`
--

LOCK TABLES `trade_requests` WRITE;
/*!40000 ALTER TABLE `trade_requests` DISABLE KEYS */;
INSERT INTO `trade_requests` VALUES (1,NULL,1,23,'expired','2026-02-07 02:59:49',NULL,'2026-02-07 06:01:49'),(2,NULL,23,1,'expired','2026-02-07 13:56:16',NULL,'2026-02-07 16:58:16'),(3,NULL,1,23,'expired','2026-02-07 14:03:01',NULL,'2026-02-07 17:05:01'),(4,NULL,23,1,'expired','2026-02-07 14:03:19',NULL,'2026-02-07 17:05:19'),(5,NULL,23,1,'expired','2026-02-07 14:37:54',NULL,'2026-02-07 17:39:54'),(6,NULL,1,23,'expired','2026-02-07 14:38:00',NULL,'2026-02-07 17:40:00'),(7,NULL,14,4,'expired','2026-02-07 14:40:30',NULL,'2026-02-07 17:42:30'),(8,NULL,4,14,'expired','2026-02-07 14:40:36',NULL,'2026-02-07 17:42:36'),(9,NULL,1,23,'expired','2026-02-07 14:55:51',NULL,'2026-02-07 17:57:51'),(10,NULL,23,1,'expired','2026-02-07 15:13:07',NULL,'2026-02-07 18:15:07'),(11,NULL,1,23,'expired','2026-02-07 15:13:21',NULL,'2026-02-07 18:15:21'),(12,NULL,1,23,'expired','2026-02-07 15:15:55',NULL,'2026-02-07 18:17:55'),(13,NULL,23,1,'expired','2026-02-07 15:16:01',NULL,'2026-02-07 18:18:01'),(14,NULL,23,1,'expired','2026-02-07 15:31:11',NULL,'2026-02-07 18:33:11'),(15,NULL,1,23,'expired','2026-02-07 15:31:28',NULL,'2026-02-07 18:33:28'),(16,NULL,23,1,'expired','2026-02-07 15:42:56',NULL,'2026-02-07 18:44:56'),(17,NULL,1,23,'expired','2026-02-07 15:43:04',NULL,'2026-02-07 18:45:04'),(18,NULL,1,23,'expired','2026-02-07 15:54:22',NULL,'2026-02-07 18:56:22'),(19,NULL,23,1,'expired','2026-02-07 15:54:33',NULL,'2026-02-07 18:56:33'),(20,NULL,1,23,'expired','2026-02-07 16:04:36',NULL,'2026-02-07 19:06:36'),(21,NULL,1,23,'expired','2026-02-07 16:07:10',NULL,'2026-02-07 19:09:10'),(22,NULL,1,23,'expired','2026-02-07 16:10:22',NULL,'2026-02-07 19:12:22'),(23,NULL,1,23,'expired','2026-02-07 16:19:01',NULL,'2026-02-07 19:21:01'),(24,NULL,1,23,'expired','2026-02-07 17:01:27',NULL,'2026-02-07 20:03:27'),(25,NULL,23,1,'expired','2026-02-07 17:01:37',NULL,'2026-02-07 20:03:37'),(26,NULL,1,23,'expired','2026-02-07 17:07:04',NULL,'2026-02-07 20:09:04'),(27,NULL,1,23,'expired','2026-02-07 17:14:53',NULL,'2026-02-07 20:16:53'),(28,NULL,1,23,'expired','2026-02-07 17:22:14',NULL,'2026-02-07 20:24:14'),(30,NULL,1,23,'expired','2026-02-07 17:29:05',NULL,'2026-02-07 20:31:05'),(31,2,1,23,'accepted','2026-02-07 17:33:44','2026-02-07 17:34:26','2026-02-07 20:35:44'),(33,4,23,1,'accepted','2026-02-07 18:07:52','2026-02-07 18:07:54','2026-02-07 21:09:52'),(35,6,1,23,'accepted','2026-02-07 18:51:24','2026-02-07 18:51:27','2026-02-07 21:53:24'),(36,7,23,1,'accepted','2026-02-07 19:02:35','2026-02-07 19:02:38','2026-02-07 22:04:35'),(37,8,23,1,'accepted','2026-02-07 19:05:43','2026-02-07 19:05:44','2026-02-07 22:07:43'),(38,9,23,1,'accepted','2026-02-07 19:58:53','2026-02-07 19:58:57','2026-02-07 23:00:53'),(39,10,1,23,'accepted','2026-02-07 20:27:44','2026-02-07 20:27:46','2026-02-07 23:29:44'),(40,11,1,23,'accepted','2026-02-07 21:12:45','2026-02-07 21:12:47','2026-02-08 00:14:45'),(41,12,23,1,'accepted','2026-02-07 21:15:19','2026-02-07 21:15:21','2026-02-08 00:17:19'),(42,13,1,23,'accepted','2026-02-07 21:19:21','2026-02-07 21:19:23','2026-02-08 00:21:21'),(43,14,23,1,'accepted','2026-02-07 21:27:40','2026-02-07 21:27:41','2026-02-08 00:29:40'),(44,15,1,23,'accepted','2026-02-07 21:29:23','2026-02-07 21:29:25','2026-02-08 00:31:23'),(45,16,1,23,'accepted','2026-02-07 21:45:41','2026-02-07 21:45:43','2026-02-08 00:47:41'),(46,17,1,23,'accepted','2026-02-07 23:51:00','2026-02-07 23:51:02','2026-02-08 02:53:00'),(47,18,1,23,'accepted','2026-02-07 23:56:16','2026-02-07 23:56:18','2026-02-08 02:58:16'),(48,19,23,1,'accepted','2026-02-07 23:59:05','2026-02-07 23:59:07','2026-02-08 03:01:05'),(49,20,1,23,'accepted','2026-02-08 00:03:04','2026-02-08 00:03:11','2026-02-08 03:05:04'),(50,21,1,23,'accepted','2026-02-08 00:05:46','2026-02-08 00:06:10','2026-02-08 03:07:46'),(51,22,23,1,'accepted','2026-02-08 01:22:19','2026-02-08 01:22:20','2026-02-08 04:24:19'),(52,NULL,23,1,'expired','2026-02-08 01:36:59',NULL,'2026-02-08 04:38:59'),(53,23,1,23,'accepted','2026-02-08 01:37:12','2026-02-08 01:37:14','2026-02-08 04:39:12'),(54,24,23,1,'accepted','2026-02-08 02:14:55','2026-02-08 02:14:57','2026-02-08 05:16:55'),(55,25,1,23,'accepted','2026-02-08 02:45:20','2026-02-08 02:45:21','2026-02-08 05:47:20'),(56,26,1,23,'accepted','2026-02-08 02:48:27','2026-02-08 02:48:29','2026-02-08 05:50:27'),(57,27,1,23,'accepted','2026-02-08 03:10:32','2026-02-08 03:10:34','2026-02-08 06:12:32'),(58,28,23,1,'accepted','2026-02-08 03:13:32','2026-02-08 03:13:54','2026-02-08 06:15:32'),(59,29,1,23,'accepted','2026-02-08 14:23:40','2026-02-08 14:23:43','2026-02-08 17:25:40'),(60,30,23,1,'accepted','2026-02-08 14:26:58','2026-02-08 14:27:00','2026-02-08 17:28:58'),(61,31,1,23,'accepted','2026-02-08 14:53:44','2026-02-08 14:53:46','2026-02-08 17:55:44'),(62,32,23,1,'accepted','2026-02-08 15:08:48','2026-02-08 15:08:49','2026-02-08 18:10:48'),(63,33,1,23,'accepted','2026-02-08 15:11:17','2026-02-08 15:11:19','2026-02-08 18:13:17'),(64,34,23,1,'accepted','2026-02-08 15:17:26','2026-02-08 15:17:27','2026-02-08 18:19:26'),(65,35,23,1,'accepted','2026-02-08 15:20:12','2026-02-08 15:20:14','2026-02-08 18:22:12'),(66,36,23,1,'accepted','2026-02-08 15:20:44','2026-02-08 15:20:46','2026-02-08 18:22:44'),(67,37,1,23,'accepted','2026-02-08 15:28:08','2026-02-08 15:28:10','2026-02-08 18:30:08'),(68,38,1,23,'accepted','2026-02-08 15:32:03','2026-02-08 15:32:05','2026-02-08 18:34:03'),(69,39,23,1,'accepted','2026-02-08 15:33:24','2026-02-08 15:33:26','2026-02-08 18:35:24'),(70,40,1,23,'accepted','2026-02-08 15:35:05','2026-02-08 15:35:07','2026-02-08 18:37:05'),(71,41,1,23,'accepted','2026-02-08 15:36:16','2026-02-08 15:36:18','2026-02-08 18:38:16'),(72,42,23,1,'accepted','2026-02-08 15:37:43','2026-02-08 15:37:45','2026-02-08 18:39:43'),(73,43,23,1,'accepted','2026-02-08 15:38:17','2026-02-08 15:38:19','2026-02-08 18:40:17'),(74,44,1,23,'accepted','2026-02-08 15:38:26','2026-02-08 15:38:27','2026-02-08 18:40:26'),(75,45,23,1,'accepted','2026-02-08 15:43:27','2026-02-08 15:43:29','2026-02-08 18:45:27'),(76,46,1,23,'accepted','2026-02-08 16:20:53','2026-02-08 16:20:56','2026-02-08 19:22:53'),(77,47,1,23,'accepted','2026-02-08 18:55:37','2026-02-08 18:55:39','2026-02-08 21:57:37'),(78,48,23,1,'accepted','2026-02-08 18:58:14','2026-02-08 18:58:15','2026-02-08 22:00:14'),(79,49,23,1,'accepted','2026-02-08 18:59:35','2026-02-08 18:59:36','2026-02-08 22:01:35'),(80,50,23,1,'accepted','2026-02-08 19:01:08','2026-02-08 19:01:10','2026-02-08 22:03:08'),(81,51,1,23,'accepted','2026-02-08 19:07:15','2026-02-08 19:07:17','2026-02-08 22:09:15'),(82,52,1,23,'accepted','2026-02-08 19:12:28','2026-02-08 19:12:30','2026-02-08 22:14:28'),(83,53,23,1,'accepted','2026-02-08 19:15:20','2026-02-08 19:15:21','2026-02-08 22:17:20'),(84,NULL,23,1,'expired','2026-02-08 19:17:37',NULL,'2026-02-08 22:19:37'),(85,54,23,1,'accepted','2026-02-08 19:20:30','2026-02-08 19:21:15','2026-02-08 22:22:30'),(86,NULL,1,23,'expired','2026-02-08 19:20:35',NULL,'2026-02-08 22:22:34'),(87,NULL,23,1,'expired','2026-02-08 19:22:29',NULL,'2026-02-08 22:24:29'),(88,55,23,1,'accepted','2026-02-08 19:25:09','2026-02-08 19:25:11','2026-02-08 22:27:09'),(89,56,1,23,'accepted','2026-02-08 19:27:22','2026-02-08 19:27:24','2026-02-08 22:29:22'),(90,57,1,23,'accepted','2026-02-08 19:34:05','2026-02-08 19:34:07','2026-02-08 22:36:05'),(91,58,1,23,'accepted','2026-02-08 20:20:26','2026-02-08 20:20:28','2026-02-08 23:22:26'),(92,59,1,23,'accepted','2026-02-08 20:22:53','2026-02-08 20:22:55','2026-02-08 23:24:53'),(93,60,1,23,'accepted','2026-02-08 20:31:08','2026-02-08 20:31:09','2026-02-08 23:33:08'),(94,61,1,23,'accepted','2026-02-08 20:39:40','2026-02-08 20:39:42','2026-02-08 23:41:40'),(95,62,1,23,'accepted','2026-02-08 20:43:41','2026-02-08 20:43:42','2026-02-08 23:45:41'),(96,63,1,23,'accepted','2026-02-08 20:45:02','2026-02-08 20:45:04','2026-02-08 23:47:02'),(97,NULL,1,23,'expired','2026-02-08 20:47:05',NULL,'2026-02-08 23:49:05'),(98,64,23,1,'accepted','2026-02-08 20:47:12','2026-02-08 20:47:15','2026-02-08 23:49:12'),(99,65,23,1,'accepted','2026-02-08 20:48:06','2026-02-08 20:48:07','2026-02-08 23:50:06'),(100,67,1,23,'accepted','2026-02-08 20:50:13','2026-02-08 20:52:08','2026-02-08 23:52:13'),(101,66,23,1,'accepted','2026-02-08 20:50:18','2026-02-08 20:50:20','2026-02-08 23:52:18'),(102,68,23,1,'accepted','2026-02-08 20:52:12','2026-02-08 20:52:14','2026-02-08 23:54:12'),(103,69,1,23,'accepted','2026-02-08 20:52:18','2026-02-08 20:52:42','2026-02-08 23:54:18'),(104,NULL,23,1,'expired','2026-02-08 20:52:45',NULL,'2026-02-08 23:54:45'),(105,70,1,23,'accepted','2026-02-08 20:55:50','2026-02-08 20:56:01','2026-02-08 23:57:50'),(106,71,1,23,'accepted','2026-02-08 20:58:14','2026-02-08 20:58:28','2026-02-09 00:00:14'),(107,72,23,1,'accepted','2026-02-08 20:58:45','2026-02-08 20:59:45','2026-02-09 00:00:45'),(108,73,1,23,'accepted','2026-02-08 20:59:51','2026-02-08 20:59:52','2026-02-09 00:01:51'),(109,74,23,1,'accepted','2026-02-08 22:21:24','2026-02-08 22:21:26','2026-02-09 01:23:24'),(110,75,1,23,'accepted','2026-02-08 23:01:34','2026-02-08 23:01:36','2026-02-09 02:03:34'),(111,76,23,1,'accepted','2026-02-08 23:02:23','2026-02-08 23:02:31','2026-02-09 02:04:23'),(112,77,1,23,'accepted','2026-02-08 23:03:25','2026-02-08 23:03:27','2026-02-09 02:05:25'),(113,78,23,1,'accepted','2026-02-08 23:26:40','2026-02-08 23:26:42','2026-02-09 02:28:40'),(114,79,1,23,'accepted','2026-02-08 23:26:46','2026-02-08 23:26:48','2026-02-09 02:28:46'),(115,80,23,1,'accepted','2026-02-08 23:26:51','2026-02-08 23:26:53','2026-02-09 02:28:51'),(116,81,1,23,'accepted','2026-02-08 23:26:57','2026-02-08 23:26:59','2026-02-09 02:28:57'),(118,83,1,23,'accepted','2026-02-09 00:05:57','2026-02-09 00:05:59','2026-02-09 03:07:57'),(120,85,1,23,'accepted','2026-02-09 01:01:06','2026-02-09 01:01:10','2026-02-09 04:03:06'),(121,86,25,30,'accepted','2026-02-09 01:14:53','2026-02-09 01:14:54','2026-02-09 04:16:53'),(122,87,23,1,'accepted','2026-02-09 02:08:54','2026-02-09 02:08:56','2026-02-09 05:10:54'),(123,88,1,4,'accepted','2026-02-09 16:59:58','2026-02-09 17:00:01','2026-02-09 20:01:58'),(124,89,1,23,'accepted','2026-02-09 22:45:13','2026-02-09 22:45:14','2026-02-10 01:47:13'),(125,90,1,23,'accepted','2026-02-09 22:48:17','2026-02-09 22:48:19','2026-02-10 01:50:17'),(126,91,23,1,'accepted','2026-02-09 22:48:48','2026-02-09 22:48:49','2026-02-10 01:50:48'),(127,92,1,23,'accepted','2026-02-09 23:15:08','2026-02-09 23:15:11','2026-02-10 02:17:08'),(128,93,23,1,'accepted','2026-02-10 12:07:04','2026-02-10 12:07:06','2026-02-10 15:09:04'),(129,94,1,23,'accepted','2026-02-10 13:20:08','2026-02-10 13:20:10','2026-02-10 16:22:08'),(130,95,23,1,'accepted','2026-02-10 23:36:45','2026-02-10 23:36:46','2026-02-11 02:38:45'),(131,96,1,23,'accepted','2026-02-11 19:00:31','2026-02-11 19:00:32','2026-02-11 22:02:31'),(132,97,23,1,'accepted','2026-02-11 21:43:48','2026-02-11 21:43:50','2026-02-12 00:45:48'),(133,98,1,23,'accepted','2026-02-12 14:07:03','2026-02-12 14:07:04','2026-02-12 17:09:03'),(134,99,23,1,'accepted','2026-02-13 00:17:05','2026-02-13 00:17:07','2026-02-13 03:19:05'),(135,100,23,1,'accepted','2026-02-13 16:56:20','2026-02-13 16:56:21','2026-02-13 19:58:20'),(136,101,1,23,'accepted','2026-02-13 16:58:59','2026-02-13 16:59:00','2026-02-13 20:00:59'),(137,102,1,23,'accepted','2026-02-13 17:02:45','2026-02-13 17:02:46','2026-02-13 20:04:45'),(138,103,1,23,'accepted','2026-02-14 10:15:25','2026-02-14 10:15:27','2026-02-14 13:17:25'),(139,104,1,23,'accepted','2026-02-15 00:15:13','2026-02-15 00:15:15','2026-02-15 03:17:13'),(141,106,1,23,'accepted','2026-02-20 17:10:13','2026-02-20 17:10:14','2026-02-20 20:12:13'),(143,108,1,32,'accepted','2026-02-24 15:12:06','2026-02-24 15:12:07','2026-02-24 18:14:06'),(144,109,1,34,'accepted','2026-02-25 00:35:50','2026-02-25 00:35:52','2026-02-25 03:37:50'),(145,110,23,1,'accepted','2026-02-25 19:47:19','2026-02-25 19:47:20','2026-02-25 22:49:19'),(148,111,25,33,'accepted','2026-02-26 14:54:27','2026-02-26 14:54:28','2026-02-26 17:56:27'),(149,112,1,23,'accepted','2026-02-26 15:10:32','2026-02-26 15:10:33','2026-02-26 18:12:32'),(150,113,23,1,'accepted','2026-02-26 16:12:22','2026-02-26 16:12:25','2026-02-26 19:14:22'),(151,114,1,23,'accepted','2026-02-26 18:40:12','2026-02-26 18:40:13','2026-02-26 21:42:12'),(152,115,1,23,'accepted','2026-03-02 18:44:58','2026-03-02 18:44:59','2026-03-02 21:46:58'),(153,116,1,25,'accepted','2026-03-06 01:14:51','2026-03-06 01:14:53','2026-03-06 04:16:51'),(154,117,1,25,'accepted','2026-03-10 15:29:47','2026-03-10 15:29:49','2026-03-10 18:31:47'),(155,118,1,25,'accepted','2026-03-11 22:54:38','2026-03-11 22:54:40','2026-03-12 01:56:38'),(156,NULL,25,23,'expired','2026-03-15 08:19:10',NULL,'2026-03-15 11:21:10'),(157,NULL,23,25,'expired','2026-03-15 16:24:24',NULL,'2026-03-15 19:26:24'),(158,NULL,23,25,'expired','2026-03-15 16:56:26',NULL,'2026-03-15 19:58:26'),(161,NULL,33,23,'expired','2026-03-15 21:01:23',NULL,'2026-03-16 00:03:23'),(162,NULL,23,25,'expired','2026-03-15 21:03:33',NULL,'2026-03-16 00:05:33'),(163,NULL,25,23,'expired','2026-03-15 21:03:43',NULL,'2026-03-16 00:05:43'),(164,NULL,25,23,'expired','2026-04-06 22:32:46',NULL,'2026-04-07 01:34:46'),(165,NULL,23,25,'expired','2026-04-06 23:10:05',NULL,'2026-04-07 02:12:05'),(166,NULL,25,23,'expired','2026-04-06 23:39:22',NULL,'2026-04-07 02:41:22'),(167,NULL,23,25,'expired','2026-04-07 00:00:53',NULL,'2026-04-07 03:02:53'),(168,NULL,25,23,'expired','2026-04-07 00:29:30',NULL,'2026-04-07 03:31:30'),(169,NULL,23,25,'expired','2026-04-07 03:34:13',NULL,'2026-04-07 06:36:13'),(170,119,25,23,'accepted','2026-04-07 03:48:20','2026-04-07 03:48:22','2026-04-07 06:50:20'),(171,120,23,25,'accepted','2026-04-08 23:08:00','2026-04-08 23:08:01','2026-04-09 02:10:00'),(172,121,23,25,'accepted','2026-04-13 00:32:50','2026-04-13 00:32:52','2026-04-13 03:34:50'),(173,122,25,23,'accepted','2026-04-14 01:59:08','2026-04-14 01:59:10','2026-04-14 05:01:08'),(174,123,25,23,'accepted','2026-04-19 06:46:22','2026-04-19 06:46:25','2026-04-19 09:48:22'),(175,124,23,2,'accepted','2026-04-27 20:06:14','2026-04-27 20:06:16','2026-04-27 23:08:14'),(176,125,1,23,'accepted','2026-05-06 15:32:22','2026-05-06 15:32:24','2026-05-06 18:34:22'),(177,126,46,1,'accepted','2026-05-06 16:11:28','2026-05-06 16:11:29','2026-05-06 19:13:28'),(178,127,1,46,'accepted','2026-05-06 16:24:22','2026-05-06 16:24:24','2026-05-06 19:26:22'),(179,128,27,25,'accepted','2026-05-10 13:32:06','2026-05-10 13:32:09','2026-05-10 16:34:06'),(180,129,23,25,'accepted','2026-05-10 17:05:34','2026-05-10 17:05:36','2026-05-10 20:07:34'),(181,130,23,25,'accepted','2026-05-10 18:59:17','2026-05-10 18:59:19','2026-05-10 22:01:17'),(182,131,23,25,'accepted','2026-05-10 19:02:05','2026-05-10 19:02:07','2026-05-10 22:04:05'),(183,132,23,25,'accepted','2026-05-10 19:11:37','2026-05-10 19:11:39','2026-05-10 22:13:37'),(184,133,23,25,'accepted','2026-05-10 19:13:51','2026-05-10 19:13:52','2026-05-10 22:15:51'),(185,134,23,25,'accepted','2026-05-10 19:15:28','2026-05-10 19:15:29','2026-05-10 22:17:28'),(186,135,23,1,'accepted','2026-05-10 19:18:26','2026-05-10 19:18:28','2026-05-10 22:20:26'),(187,136,25,23,'accepted','2026-05-10 19:20:29','2026-05-10 19:20:31','2026-05-10 22:22:29'),(188,137,23,25,'accepted','2026-05-10 19:22:45','2026-05-10 19:22:47','2026-05-10 22:24:45'),(189,138,25,23,'accepted','2026-05-10 19:25:18','2026-05-10 19:25:20','2026-05-10 22:27:18'),(190,139,25,23,'accepted','2026-05-10 21:10:00','2026-05-10 21:10:02','2026-05-11 00:12:00'),(191,140,27,23,'accepted','2026-05-11 01:22:17','2026-05-11 01:22:19','2026-05-11 04:24:17'),(192,141,23,27,'accepted','2026-05-11 01:42:19','2026-05-11 01:42:20','2026-05-11 04:44:19'),(193,142,23,27,'accepted','2026-05-11 01:44:52','2026-05-11 01:44:54','2026-05-11 04:46:52'),(194,143,27,23,'accepted','2026-05-11 01:45:10','2026-05-11 01:45:12','2026-05-11 04:47:10'),(195,144,23,27,'accepted','2026-05-11 01:45:23','2026-05-11 01:45:25','2026-05-11 04:47:23'),(196,NULL,1,23,'expired','2026-05-16 00:00:12',NULL,'2026-05-16 03:02:12'),(197,NULL,1,32,'expired','2026-05-16 00:09:50',NULL,'2026-05-16 03:11:50'),(198,NULL,23,1,'expired','2026-05-19 21:43:38',NULL,'2026-05-20 00:45:38'),(199,145,1,25,'accepted','2026-05-21 20:39:45','2026-05-21 20:39:46','2026-05-21 23:41:45'),(200,146,1,25,'accepted','2026-05-22 00:28:57','2026-05-22 00:29:02','2026-05-22 03:30:57'),(201,147,1,25,'accepted','2026-05-30 21:15:36','2026-05-30 21:15:37','2026-05-31 00:17:36'),(202,NULL,25,1,'expired','2026-05-31 03:16:40',NULL,'2026-05-31 06:18:40'),(203,148,1,23,'accepted','2026-06-19 02:04:03','2026-06-19 02:04:04','2026-06-19 05:06:03'),(204,NULL,1,23,'expired','2026-06-20 22:44:46',NULL,'2026-06-21 01:46:46'),(205,NULL,23,1,'expired','2026-06-20 22:46:40',NULL,'2026-06-21 01:48:40'),(206,NULL,1,23,'expired','2026-06-20 22:46:47',NULL,'2026-06-21 01:48:47'),(207,NULL,23,1,'expired','2026-06-20 22:52:29',NULL,'2026-06-21 01:54:29'),(208,NULL,23,1,'expired','2026-06-20 23:20:13',NULL,'2026-06-21 02:22:13'),(209,NULL,1,23,'expired','2026-06-20 23:20:16',NULL,'2026-06-21 02:22:16'),(210,NULL,23,1,'expired','2026-06-20 23:53:15',NULL,'2026-06-21 02:55:15'),(211,NULL,1,23,'expired','2026-06-21 00:20:07',NULL,'2026-06-21 03:22:07'),(212,149,1,23,'accepted','2026-06-21 00:50:01','2026-06-21 00:50:03','2026-06-21 03:52:01'),(213,150,23,1,'accepted','2026-06-23 02:06:37','2026-06-23 02:06:38','2026-06-23 05:08:37'),(214,151,23,1,'accepted','2026-06-23 02:07:03','2026-06-23 02:07:04','2026-06-23 05:09:03'),(215,152,1,23,'accepted','2026-06-23 02:16:40','2026-06-23 02:16:42','2026-06-23 05:18:40'),(216,NULL,25,1,'expired','2026-06-25 05:12:52',NULL,'2026-06-25 08:14:52');
/*!40000 ALTER TABLE `trade_requests` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `trade_sessions`
--

DROP TABLE IF EXISTS `trade_sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `trade_sessions` (
  `trade_session_id` int unsigned NOT NULL AUTO_INCREMENT,
  `player1_id` bigint unsigned NOT NULL,
  `player2_id` bigint unsigned NOT NULL,
  `player1_ready` tinyint(1) DEFAULT '0',
  `player2_ready` tinyint(1) DEFAULT '0',
  `status` enum('active','completed','cancelled') COLLATE utf8mb4_unicode_ci DEFAULT 'active',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `completed_at` timestamp NULL DEFAULT NULL,
  `player1_gold_offer` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Gold que player1 oferece na troca',
  `player2_gold_offer` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'Gold que player2 oferece na troca',
  PRIMARY KEY (`trade_session_id`),
  KEY `idx_player1` (`player1_id`),
  KEY `idx_player2` (`player2_id`),
  KEY `idx_status` (`status`),
  CONSTRAINT `trade_sessions_ibfk_1` FOREIGN KEY (`player1_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,
  CONSTRAINT `trade_sessions_ibfk_2` FOREIGN KEY (`player2_id`) REFERENCES `players` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=153 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `trade_sessions`
--

LOCK TABLES `trade_sessions` WRITE;
/*!40000 ALTER TABLE `trade_sessions` DISABLE KEYS */;
INSERT INTO `trade_sessions` VALUES (2,1,23,0,0,'cancelled','2026-02-07 17:34:26','2026-02-07 17:55:19',0,0),(4,23,1,0,0,'cancelled','2026-02-07 18:07:54','2026-02-07 18:50:53',0,0),(6,1,23,0,0,'cancelled','2026-02-07 18:51:27','2026-02-07 19:02:13',0,0),(7,23,1,0,0,'cancelled','2026-02-07 19:02:38','2026-02-07 19:05:26',0,0),(8,23,1,0,0,'cancelled','2026-02-07 19:05:44','2026-02-07 19:58:17',0,0),(9,23,1,0,0,'cancelled','2026-02-07 19:58:57','2026-02-07 20:26:06',0,0),(10,1,23,0,0,'cancelled','2026-02-07 20:27:46','2026-02-07 21:12:25',0,0),(11,1,23,0,0,'cancelled','2026-02-07 21:12:47','2026-02-07 21:14:49',0,0),(12,23,1,0,0,'cancelled','2026-02-07 21:15:21','2026-02-07 21:18:57',0,0),(13,1,23,0,0,'cancelled','2026-02-07 21:19:23','2026-02-07 21:27:20',0,0),(14,23,1,0,0,'cancelled','2026-02-07 21:27:41','2026-02-07 21:28:54',0,0),(15,1,23,0,0,'cancelled','2026-02-07 21:29:25','2026-02-07 21:45:18',0,0),(16,1,23,0,0,'cancelled','2026-02-07 21:45:43','2026-02-07 23:50:30',0,0),(17,1,23,0,0,'cancelled','2026-02-07 23:51:02','2026-02-07 23:55:42',0,0),(18,1,23,0,0,'cancelled','2026-02-07 23:56:18','2026-02-07 23:58:36',0,0),(19,23,1,0,0,'cancelled','2026-02-07 23:59:07','2026-02-08 00:02:37',0,0),(20,1,23,0,0,'cancelled','2026-02-08 00:03:11','2026-02-08 00:05:22',0,0),(21,1,23,0,0,'cancelled','2026-02-08 00:06:10','2026-02-08 01:21:48',0,0),(22,23,1,0,0,'cancelled','2026-02-08 01:22:20','2026-02-08 01:36:23',0,0),(23,1,23,0,0,'cancelled','2026-02-08 01:37:14','2026-02-08 02:14:30',0,0),(24,23,1,0,0,'cancelled','2026-02-08 02:14:57','2026-02-08 02:44:46',0,0),(25,1,23,0,0,'cancelled','2026-02-08 02:45:21','2026-02-08 02:47:59',0,0),(26,1,23,0,0,'cancelled','2026-02-08 02:48:29','2026-02-08 03:09:47',0,0),(27,1,23,0,0,'cancelled','2026-02-08 03:10:34','2026-02-08 03:12:44',0,0),(28,23,1,0,0,'cancelled','2026-02-08 03:13:54','2026-02-08 14:23:02',0,0),(29,1,23,0,0,'cancelled','2026-02-08 14:23:43','2026-02-08 14:26:30',0,0),(30,23,1,0,0,'cancelled','2026-02-08 14:27:00','2026-02-08 14:53:12',0,0),(31,1,23,0,0,'completed','2026-02-08 14:53:46','2026-02-08 14:54:19',0,0),(32,23,1,0,0,'cancelled','2026-02-08 15:08:49','2026-02-08 15:10:48',0,0),(33,1,23,0,0,'cancelled','2026-02-08 15:11:19','2026-02-08 15:17:05',0,0),(34,23,1,0,0,'cancelled','2026-02-08 15:17:27','2026-02-08 15:18:26',0,0),(35,23,1,0,0,'cancelled','2026-02-08 15:20:14','2026-02-08 15:20:39',0,0),(36,23,1,0,0,'cancelled','2026-02-08 15:20:46','2026-02-08 15:27:36',0,0),(37,1,23,0,0,'cancelled','2026-02-08 15:28:10','2026-02-08 15:31:36',0,0),(38,1,23,0,0,'cancelled','2026-02-08 15:32:05','2026-02-08 15:32:52',0,0),(39,23,1,0,0,'cancelled','2026-02-08 15:33:26','2026-02-08 15:33:33',0,0),(40,1,23,0,0,'cancelled','2026-02-08 15:35:07','2026-02-08 15:36:16',0,0),(41,1,23,0,0,'cancelled','2026-02-08 15:36:18','2026-02-08 15:37:43',0,0),(42,23,1,0,0,'cancelled','2026-02-08 15:37:45','2026-02-08 15:38:10',0,0),(43,23,1,0,0,'cancelled','2026-02-08 15:38:19','2026-02-08 15:38:21',0,0),(44,1,23,0,0,'cancelled','2026-02-08 15:38:27','2026-02-08 15:42:55',0,0),(45,23,1,0,0,'cancelled','2026-02-08 15:43:29','2026-02-08 15:45:00',0,0),(46,1,23,0,0,'cancelled','2026-02-08 16:20:56','2026-02-08 18:55:06',0,0),(47,1,23,0,0,'cancelled','2026-02-08 18:55:39','2026-02-08 18:57:47',300000,0),(48,23,1,0,0,'completed','2026-02-08 18:58:15','2026-02-08 18:59:26',0,500),(49,23,1,0,0,'completed','2026-02-08 18:59:36','2026-02-08 19:00:56',0,5000),(50,23,1,0,0,'cancelled','2026-02-08 19:01:10','2026-02-08 19:06:46',0,0),(51,1,23,0,0,'completed','2026-02-08 19:07:17','2026-02-08 19:07:42',2500,500),(52,1,23,0,0,'completed','2026-02-08 19:12:30','2026-02-08 19:13:26',1750,3250),(53,23,1,0,0,'completed','2026-02-08 19:15:21','2026-02-08 19:15:37',0,50000),(54,23,1,0,0,'completed','2026-02-08 19:21:15','2026-02-08 19:21:50',2500,0),(55,23,1,0,0,'cancelled','2026-02-08 19:25:11','2026-02-08 19:25:56',0,0),(56,1,23,0,0,'cancelled','2026-02-08 19:27:24','2026-02-08 19:27:35',0,0),(57,1,23,0,0,'completed','2026-02-08 19:34:07','2026-02-08 19:34:29',0,55),(58,1,23,0,0,'completed','2026-02-08 20:20:28','2026-02-08 20:20:48',0,0),(59,1,23,0,0,'completed','2026-02-08 20:22:55','2026-02-08 20:23:34',0,0),(60,1,23,0,0,'completed','2026-02-08 20:31:09','2026-02-08 20:31:22',0,0),(61,1,23,0,0,'completed','2026-02-08 20:39:42','2026-02-08 20:39:58',0,0),(62,1,23,0,0,'completed','2026-02-08 20:43:42','2026-02-08 20:43:59',0,0),(63,1,23,0,0,'completed','2026-02-08 20:45:04','2026-02-08 20:45:19',0,0),(64,23,1,0,0,'completed','2026-02-08 20:47:15','2026-02-08 20:47:29',0,0),(65,23,1,0,0,'completed','2026-02-08 20:48:07','2026-02-08 20:48:19',0,0),(66,23,1,0,0,'completed','2026-02-08 20:50:20','2026-02-08 20:50:38',0,0),(67,1,23,0,0,'cancelled','2026-02-08 20:52:08','2026-02-08 20:52:09',0,0),(68,23,1,0,0,'cancelled','2026-02-08 20:52:14','2026-02-08 20:52:16',0,0),(69,1,23,0,0,'cancelled','2026-02-08 20:52:42','2026-02-08 20:52:43',0,0),(70,1,23,0,0,'cancelled','2026-02-08 20:56:01','2026-02-08 20:57:46',0,0),(71,1,23,0,0,'cancelled','2026-02-08 20:58:28','2026-02-08 20:58:40',0,0),(72,23,1,0,0,'cancelled','2026-02-08 20:59:45','2026-02-08 20:59:47',0,0),(73,1,23,0,0,'cancelled','2026-02-08 20:59:52','2026-02-08 22:20:36',0,0),(74,23,1,0,0,'cancelled','2026-02-08 22:21:26','2026-02-08 22:21:29',0,0),(75,1,23,0,0,'cancelled','2026-02-08 23:01:36','2026-02-08 23:01:38',0,0),(76,23,1,0,0,'cancelled','2026-02-08 23:02:31','2026-02-08 23:03:23',0,0),(77,1,23,0,0,'completed','2026-02-08 23:03:27','2026-02-08 23:04:31',0,0),(78,23,1,0,0,'cancelled','2026-02-08 23:26:42','2026-02-08 23:26:43',0,0),(79,1,23,0,0,'cancelled','2026-02-08 23:26:48','2026-02-08 23:26:48',0,0),(80,23,1,0,0,'cancelled','2026-02-08 23:26:53','2026-02-08 23:26:54',0,0),(81,1,23,0,0,'cancelled','2026-02-08 23:26:59','2026-02-08 23:27:00',0,0),(83,1,23,0,0,'cancelled','2026-02-09 00:05:59','2026-02-09 00:06:27',0,0),(85,1,23,0,0,'completed','2026-02-09 01:01:10','2026-02-09 01:01:19',0,0),(86,25,30,0,0,'cancelled','2026-02-09 01:14:54','2026-02-09 01:14:56',0,0),(87,23,1,0,0,'cancelled','2026-02-09 02:08:56','2026-02-09 02:08:58',0,0),(88,1,4,0,0,'completed','2026-02-09 17:00:01','2026-02-09 17:00:22',0,0),(89,1,23,0,0,'completed','2026-02-09 22:45:14','2026-02-09 22:45:58',10000,0),(90,1,23,0,0,'cancelled','2026-02-09 22:48:19','2026-02-09 22:48:20',0,0),(91,23,1,0,0,'completed','2026-02-09 22:48:49','2026-02-09 22:49:10',0,0),(92,1,23,0,0,'completed','2026-02-09 23:15:11','2026-02-09 23:16:07',12500,725),(93,23,1,0,0,'cancelled','2026-02-10 12:07:06','2026-02-10 13:02:01',0,0),(94,1,23,0,0,'completed','2026-02-10 13:20:10','2026-02-10 13:20:32',1750,0),(95,23,1,0,0,'completed','2026-02-10 23:36:46','2026-02-10 23:37:16',0,11200),(96,1,23,0,0,'completed','2026-02-11 19:00:32','2026-02-11 19:00:56',0,0),(97,23,1,0,0,'completed','2026-02-11 21:43:50','2026-02-11 21:44:28',12500,0),(98,1,23,0,0,'completed','2026-02-12 14:07:04','2026-02-12 14:07:26',0,0),(99,23,1,0,0,'cancelled','2026-02-13 00:17:07','2026-02-13 00:17:23',0,0),(100,23,1,0,0,'completed','2026-02-13 16:56:21','2026-02-13 16:57:59',0,1270),(101,1,23,0,0,'completed','2026-02-13 16:59:00','2026-02-13 17:01:33',0,0),(102,1,23,0,0,'completed','2026-02-13 17:02:46','2026-02-13 17:03:00',0,0),(103,1,23,0,0,'completed','2026-02-14 10:15:27','2026-02-14 10:15:55',100,0),(104,1,23,0,0,'completed','2026-02-15 00:15:15','2026-02-15 00:15:32',0,0),(106,1,23,0,0,'cancelled','2026-02-20 17:10:14','2026-02-20 17:10:45',0,0),(108,1,32,0,0,'completed','2026-02-24 15:12:07','2026-02-24 15:12:25',0,0),(109,1,34,0,0,'completed','2026-02-25 00:35:52','2026-02-25 00:36:11',0,0),(110,23,1,0,0,'cancelled','2026-02-25 19:47:20','2026-02-25 19:47:26',0,0),(111,25,33,0,0,'cancelled','2026-02-26 14:54:28','2026-02-26 14:54:31',0,0),(112,1,23,0,0,'completed','2026-02-26 15:10:33','2026-02-26 15:10:46',0,0),(113,23,1,0,0,'cancelled','2026-02-26 16:12:25','2026-02-26 16:44:59',0,0),(114,1,23,0,0,'cancelled','2026-02-26 18:40:13','2026-02-26 18:40:15',0,0),(115,1,23,0,0,'completed','2026-03-02 18:44:59','2026-03-02 18:45:21',13500,0),(116,1,25,0,0,'completed','2026-03-06 01:14:53','2026-03-06 01:15:14',0,0),(117,1,25,0,0,'completed','2026-03-10 15:29:49','2026-03-10 15:30:17',0,0),(118,1,25,0,0,'cancelled','2026-03-11 22:54:40','2026-03-11 22:54:50',0,0),(119,25,23,0,0,'cancelled','2026-04-07 03:48:22','2026-04-07 03:48:34',0,0),(120,23,25,0,0,'completed','2026-04-08 23:08:01','2026-04-08 23:08:13',0,0),(121,23,25,0,0,'completed','2026-04-13 00:32:52','2026-04-13 00:33:12',20000,0),(122,25,23,0,0,'completed','2026-04-14 01:59:10','2026-04-14 01:59:24',0,0),(123,25,23,0,0,'completed','2026-04-19 06:46:25','2026-04-19 06:47:19',0,0),(124,23,2,0,0,'completed','2026-04-27 20:06:16','2026-04-27 20:06:26',0,0),(125,1,23,0,0,'completed','2026-05-06 15:32:24','2026-05-06 15:32:37',0,0),(126,46,1,0,0,'completed','2026-05-06 16:11:29','2026-05-06 16:11:43',0,0),(127,1,46,0,0,'completed','2026-05-06 16:24:24','2026-05-06 16:24:50',99999,0),(128,27,25,0,0,'cancelled','2026-05-10 13:32:09','2026-05-10 13:32:20',0,0),(129,23,25,0,0,'completed','2026-05-10 17:05:36','2026-05-10 17:05:47',0,0),(130,23,25,1,0,'cancelled','2026-05-10 18:59:19','2026-05-10 18:59:46',0,0),(131,23,25,0,0,'cancelled','2026-05-10 19:02:07','2026-05-10 19:10:55',0,0),(132,23,25,0,0,'cancelled','2026-05-10 19:11:39','2026-05-10 19:13:17',0,0),(133,23,25,0,0,'cancelled','2026-05-10 19:13:52','2026-05-10 19:14:53',0,0),(134,23,25,0,0,'cancelled','2026-05-10 19:15:29','2026-05-10 19:17:27',0,0),(135,23,1,0,0,'cancelled','2026-05-10 19:18:28','2026-05-10 19:18:33',0,0),(136,25,23,0,0,'cancelled','2026-05-10 19:20:31','2026-05-10 19:22:01',0,0),(137,23,25,0,0,'cancelled','2026-05-10 19:22:47','2026-05-10 19:23:56',0,0),(138,25,23,0,0,'cancelled','2026-05-10 19:25:20','2026-05-10 19:28:54',0,0),(139,25,23,0,0,'completed','2026-05-10 21:10:02','2026-05-10 21:10:27',0,0),(140,27,23,0,0,'completed','2026-05-11 01:22:19','2026-05-11 01:22:46',0,0),(141,23,27,0,0,'cancelled','2026-05-11 01:42:20','2026-05-11 01:42:24',0,0),(142,23,27,0,0,'cancelled','2026-05-11 01:44:54','2026-05-11 01:44:55',0,0),(143,27,23,0,0,'cancelled','2026-05-11 01:45:12','2026-05-11 01:45:17',0,0),(144,23,27,0,0,'cancelled','2026-05-11 01:45:25','2026-05-11 01:45:27',0,0),(145,1,25,0,0,'completed','2026-05-21 20:39:46','2026-05-21 20:40:02',0,0),(146,1,25,0,0,'completed','2026-05-22 00:29:02','2026-05-22 00:29:16',0,0),(147,1,25,0,0,'cancelled','2026-05-30 21:15:37','2026-05-30 21:15:40',0,0),(148,1,23,0,0,'cancelled','2026-06-19 02:04:04','2026-06-19 02:04:06',0,0),(149,1,23,0,0,'cancelled','2026-06-21 00:50:03','2026-06-21 00:50:05',0,0),(150,23,1,0,0,'completed','2026-06-23 02:06:38','2026-06-23 02:06:46',0,0),(151,23,1,0,0,'completed','2026-06-23 02:07:04','2026-06-23 02:07:14',0,0),(152,1,23,0,0,'completed','2026-06-23 02:16:42','2026-06-23 02:16:55',0,0);
/*!40000 ALTER TABLE `trade_sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping events for database 'umbra_eternum'
--

--
-- Dumping routines for database 'umbra_eternum'
--
/*!50003 DROP PROCEDURE IF EXISTS `ApplyGuildSchemaHardening` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `ApplyGuildSchemaHardening`()
BEGIN
    -- ------------------------------------------------------------------------
    -- 1) guilds.guild_tag UNIQUE
    -- ------------------------------------------------------------------------
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guilds'
          AND COLUMN_NAME = 'guild_tag'
    ) AND NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guilds'
          AND INDEX_NAME = 'ux_guild_tag'
    ) THEN
        SET @sql = 'ALTER TABLE guilds ADD UNIQUE KEY ux_guild_tag (guild_tag)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 2) remover indice redundante idx_guild_name
    -- ------------------------------------------------------------------------
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guilds'
          AND INDEX_NAME = 'idx_guild_name'
    ) THEN
        SET @sql = 'ALTER TABLE guilds DROP INDEX idx_guild_name';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 3) guild_invites.status NOT NULL DEFAULT 'pending'
    -- ------------------------------------------------------------------------
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_invites'
          AND COLUMN_NAME = 'status'
    ) THEN
        SET @sql = "ALTER TABLE guild_invites MODIFY status ENUM('pending','accepted','declined','expired','cancelled') NOT NULL DEFAULT 'pending'";
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    -- ------------------------------------------------------------------------
    -- 4) anti-duplicidade de convite pendente
    --    coluna gerada pending_invited_player_id + unique (guild_id, pending_invited_player_id)
    -- ------------------------------------------------------------------------
    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.TABLES
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_invites'
    ) AND NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_invites'
          AND COLUMN_NAME = 'pending_invited_player_id'
    ) THEN
        SET @sql = "ALTER TABLE guild_invites ADD COLUMN pending_invited_player_id BIGINT UNSIGNED GENERATED ALWAYS AS (CASE WHEN status = 'pending' THEN invited_player_id ELSE NULL END) STORED";
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;

    IF EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_invites'
          AND COLUMN_NAME = 'pending_invited_player_id'
    ) AND NOT EXISTS (
        SELECT 1 FROM INFORMATION_SCHEMA.STATISTICS
        WHERE TABLE_SCHEMA = DATABASE()
          AND TABLE_NAME = 'guild_invites'
          AND INDEX_NAME = 'ux_guild_pending_invite'
    ) THEN
        SET @sql = 'CREATE UNIQUE INDEX ux_guild_pending_invite ON guild_invites (guild_id, pending_invited_player_id)';
        PREPARE stmt FROM @sql; EXECUTE stmt; DEALLOCATE PREPARE stmt;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `calculate_skill_points` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `calculate_skill_points`(IN p_player_id BIGINT UNSIGNED)
BEGIN
    DECLARE v_level INT;
    DECLARE v_total_points INT;
    DECLARE v_spent_points INT;
    
    -- Obter nível do jogador
    SELECT `level` INTO v_level FROM `players` WHERE `id` = p_player_id;
    
    -- Calcular pontos totais baseado no nível
    SELECT COALESCE(cumulative_points, 0) INTO v_total_points 
    FROM `skill_points_per_level` 
    WHERE `level` = v_level;
    
    -- Calcular pontos gastos
    SELECT COALESCE(SUM(s.skill_cost * ps.current_rank), 0) INTO v_spent_points
    FROM `player_skills` ps
    JOIN `skills` s ON ps.skill_id = s.skill_id
    WHERE ps.player_id = p_player_id;
    
    -- Atualizar ou inserir registro
    INSERT INTO `player_skill_points` (`player_id`, `total_points_earned`, `points_spent`, `points_available`)
    VALUES (p_player_id, v_total_points, v_spent_points, v_total_points - v_spent_points)
    ON DUPLICATE KEY UPDATE
        total_points_earned = v_total_points,
        points_spent = v_spent_points,
        points_available = v_total_points - v_spent_points;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `CleanExpiredSocialRequests` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `CleanExpiredSocialRequests`()
BEGIN
    -- Limpar convites de grupo expirados
    UPDATE party_invites 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar solicitações de troca expiradas
    UPDATE trade_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar solicitações de amizade expiradas
    UPDATE friend_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
    
    -- Limpar desafios de duelo expirados
    UPDATE duel_requests 
    SET status = 'expired' 
    WHERE status = 'pending' 
      AND expires_at IS NOT NULL 
      AND expires_at < NOW();
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `cleanup_expired_buffs` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `cleanup_expired_buffs`()
BEGIN
    DELETE FROM `active_buffs` WHERE `expires_at` < NOW(3) AND `is_permanent` = 0;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `cleanup_expired_cooldowns` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `cleanup_expired_cooldowns`()
BEGIN
    DELETE FROM `player_cooldowns` WHERE `expires_at` < NOW(3);
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `cleanup_expired_dots` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_0900_ai_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `cleanup_expired_dots`()
BEGIN
    DELETE FROM `active_dots` WHERE `expires_at` < NOW(3);
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-07-09 18:52:10
