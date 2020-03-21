<?

require('../includes/utils.inc.php');

// mac of the module to remove from modules table
$mac = $_REQUEST['mac'];
if(!$mac) {
  returnError("Missing 'mac' parameter.");
}

$mysqli = connect();
$stmt =  $mysqli->stmt_init();

// First check that api_key is valid by fetching the user_id that we'll need to insert
$stmt->prepare('delete from xiot_my where mac = ?') OR returnError("Invalid delete statement");
$stmt->bind_param("s", $mac);
$stmt->execute();
$stmt->close();
$mysqli->commit();
$mysqli->close();

?>