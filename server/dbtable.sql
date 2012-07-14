CREATE TABLE IF NOT EXISTS `#db#` (
  `name` varchar(50) NOT NULL,
  `password` varchar(40) NOT NULL,
  `tcp_ip` varchar(100) NOT NULL,
  `tcp_port` int(11) NOT NULL,
  `pubkey` varchar(300) NOT NULL,
  `dateOfRegistration` datetime NOT NULL,
  `dateOfLogin` datetime NOT NULL,
  `dateOfLast` datetime NOT NULL,
  `logged` int(1) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;
