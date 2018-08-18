
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;


-- --------------------------------------------------------
--
-- Table structure for table `xiot_user`
--

CREATE TABLE `xiot_user` (
  `userid` int(11) NOT NULL,
  `apikey` char(36) NOT NULL,
  `lastname` tinytext NOT NULL,
  `firstname` tinytext NOT NULL,
  `email` tinytext NOT NULL,
  `lastUpdated` datetime NOT NULL,
  `created` datetime NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '1'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Indexes for table `xiot_user`
--
ALTER TABLE `xiot_user`
  ADD PRIMARY KEY (`userid`),
  ADD UNIQUE KEY `apikey` (`apikey`);

--
-- AUTO_INCREMENT for table `xiot_user`
--
ALTER TABLE `xiot_user`
  MODIFY `userid` int(11) NOT NULL AUTO_INCREMENT;COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
