SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for account
-- ----------------------------
CREATE TABLE `account` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `login` text NOT NULL,
  `pwhash` varchar(40) NOT NULL,
  `level` smallint(6) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=0 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ban_account
-- ----------------------------
CREATE TABLE `ban_account` (
  `account_id` int(11) unsigned NOT NULL,
  `bandate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `unbandate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `bannedby` varchar(50) NOT NULL DEFAULT 'SERVER',
  `reason` varchar(255) NOT NULL DEFAULT 'No reason set.',
  PRIMARY KEY (`account_id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for ban_ip
-- ----------------------------
CREATE TABLE `ban_ip` (
  `ip` varchar(32) NOT NULL DEFAULT '0.0.0.0',
  `bandate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `unbandate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `bannedby` varchar(50) NOT NULL DEFAULT 'SERVER',
  `reason` varchar(255) NOT NULL DEFAULT 'No reason set.',
  PRIMARY KEY (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for channel
-- ----------------------------
CREATE TABLE `channel` (
  `id` int(10) unsigned NOT NULL,
  `name` text NOT NULL,
  `password` text NOT NULL,
  `join_level` smallint(6) NOT NULL COMMENT 'Minimum level required to join the channel',
  KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for chat_history
-- ----------------------------
CREATE TABLE `chat_history` (
  `id` bigint(10) unsigned NOT NULL AUTO_INCREMENT,
  `channel` text NOT NULL,
  `name` text NOT NULL,
  `message` text NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=0 DEFAULT CHARSET=utf8;
