<?

error_reporting(E_ALL);
ini_set("display_errors", 1);
ini_set('display_startup_errors', 1);

require('../../includes/utils.inc.php');

// Ip to lookup. TODO : Check if HTTP_X_FORWARDED_FOR and use it ?
$gatewayIp = $_SERVER['REMOTE_ADDR'];

// uid
if(isset($_REQUEST['apikey'])) {
  $apikey = $_REQUEST['apikey'];
} else {
  die("Missing 'apikey' parameter.");
}

// ip to register
if(isset($_REQUEST['ip'])) {
  $localIp = $_REQUEST['ip'];
} else {
  die("Missing 'ip' parameter.");
}

// name of the module
if(isset($_REQUEST['name'])) {
  $name = $_REQUEST['name'];
} else {
  die("Missing 'name' parameter.");
}

// mac address of the module
if(isset($_REQUEST['mac'])) {
  $mac = $_REQUEST['mac'];
} else {
  die("Missing 'mac' parameter.");
}

$mysqli = connect();
$stmt =  $mysqli->stmt_init();

// First check that api_key is valid by fetching the user_id that we'll need to insert

$stmt->prepare('select userid from xiot_user where apikey = ? and enabled = 1') OR die("Invalid select user statement");
$stmt->bind_param("s", $apikey);
$stmt->execute();
$stmt->bind_result($userid);

$users = array();
while($row = $stmt->fetch()) {
  array_push($users, array("userid" => $userid));
}

$stmt->free_result();
$stmt->close();

if(count($users) != 1) {
  die("Invalid Api Key");
}

$userid = $users[0]['userid'];

// Delete previous record with same MAC address
$stmt =  $mysqli->stmt_init();
$stmt->prepare('delete from xiot_my where mac = ? ') OR die("Invalid delete MAC statement");
$stmt->bind_param("s", $mac);
$stmt->execute();
$stmt->close();

// insert new record
$stmt =  $mysqli->stmt_init();
$stmt->prepare('INSERT INTO xiot_my (mac, name, gateway_ip, local_ip, userid, date) ' .
                     ' VALUES (?, ?, ?, ?, ?, now()); ') OR die("Invalid insert statement");
$stmt->bind_param("ssssi", $mac, $name, $gatewayIp, $localIp, $userid);
if(!$stmt->execute()) {
  echo "Failed: " . mysqli_error($mysqli) ;
}
$stmt->close();
$mysqli->commit();
$mysqli->close();


?>