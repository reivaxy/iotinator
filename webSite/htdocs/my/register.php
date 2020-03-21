<?

require('../includes/utils.inc.php');
$header = apache_request_headers();

// Ip to lookup. TODO : Check if HTTP_X_FORWARDED_FOR and use it ?
$gatewayIp = $_SERVER['REMOTE_ADDR'];

$payload = file_get_contents('php://input');
$json = json_decode($payload, true);

// ip to register
$localIp = $json['ip'];
// name of the module
$name = $json['name'];
// mac address of the module
$mac = $json['mac'];
// API KEY to check registration
$apikey = $json['apikey'];

$headerApiKey = $header['apikey'];
if($headerApiKey) {
  $apikey = $headerApiKey;
}

if(!$localIp || !$name || !$mac || !$apikey) {
  returnError("Bad payload.");
}

$mysqli = connect();
$stmt =  $mysqli->stmt_init();

// First check that api_key is valid by fetching the user_id that we'll need to insert

$stmt->prepare('select userid from xiot_user where apikey = ? and enabled = 1') OR returnError("Invalid select user statement");
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
  returnError("Invalid Api Key");
}

$userid = $users[0]['userid'];

// Delete previous record with same MAC address
$stmt =  $mysqli->stmt_init();
$stmt->prepare('delete from xiot_my where mac = ? ') OR returnError("Invalid delete MAC statement");
$stmt->bind_param("s", $mac);
$stmt->execute();
$stmt->close();

// insert new record
$stmt =  $mysqli->stmt_init();
$stmt->prepare('INSERT INTO xiot_my (mac, name, gateway_ip, local_ip, userid, date) ' .
                     ' VALUES (?, ?, ?, ?, ?, now()); ') OR returnError("Invalid insert statement");
$stmt->bind_param("ssssi", $mac, $name, $gatewayIp, $localIp, $userid);
if(!$stmt->execute()) {
  returnError("Failed: " . mysqli_error($mysqli));
}
$stmt->close();
$mysqli->commit();
$mysqli->close();

// Registration is ok, return the manifest for the webApp 
// For now, it's hardcoded. Could generate a manifest exposing all available apps with their
// dependencies

//echo "{}";
echo file_get_contents("../appGLA/webpack-assets.json");
?>
