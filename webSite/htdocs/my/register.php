<?

ini_set("display_errors", 1);
require('../includes/utils.inc.php');

// uid
if(isset($_REQUEST['uid'])) {
  $uid = $_REQUEST['uid'];
} else {
  die("Missing 'uid' parameter.");
}

// ip to register
if(isset($_REQUEST['ip'])) {
  $gatewayIp = $_REQUEST['ip'];
} else {
  die("Missing 'ip' parameter.");
}

// Also mac and name, etc

?>