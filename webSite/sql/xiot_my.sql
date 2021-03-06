
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Table structure for table `xiot_my`
--

CREATE TABLE `xiot_my` (
  `userid` int(11) NOT NULL,
  `mac` varchar(20) NOT NULL,
  `name` varchar(30) NOT NULL,
  `gateway_ip` varchar(16) NOT NULL,
  `local_ip` varchar(16) NOT NULL,
  `date` datetime NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Indexes for table `my`
--
ALTER TABLE `xiot_my`
  ADD UNIQUE KEY `mac` (`mac`),
  ADD KEY `gateway` (`gateway_ip`);
COMMIT;

ALTER TABLE `xiot_my` ADD UNIQUE `mac_gateway` (`mac`, `gateway_ip`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
