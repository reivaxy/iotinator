
<?

require('utils.inc.php');

function getList() {
    
  // Ip to lookup. TODO : Check if HTTP_X_FORWARDED_FOR and use it ?
  $gatewayIp = $_SERVER['REMOTE_ADDR'];
  
  $mysqli = connect();
  $stmt =  $mysqli->stmt_init();
  $stmt->prepare('select mac, name, local_ip, date from xiot_my where gateway_ip = ? order by local_ip ') OR die("Invalid statement");
  
  $stmt->bind_param("s", $gatewayIp);
  $stmt->execute();
  
  $stmt->bind_result($mac, $name, $localIp, $date);
  
  $modules = array();
  // Copy into an array to finish all DB related stuff before displaying the table.
  while($row = $stmt->fetch()) {
    array_push($modules, array("date" => $date, "mac" => $mac, "name" => $name,
                 "localIp" => $localIp));
  
  }
  $stmt->free_result();
  $stmt->close();
  // If only one result, redirect browser to this ip.
  // Otherwise display the list with links to each
  
  return $modules;
}
?>
