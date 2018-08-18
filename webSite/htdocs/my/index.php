<html>
  <head>
  <!--  todo-->
  </head>
    <body>
      <title>My Iotinator</title>
          

<?

ini_set("display_errors", 1);
require('../includes/utils.inc.php');

// ip to look up
if(isset($_REQUEST['ip'])) {
  $gatewayIp = $_REQUEST['ip'];
} else {
  die("Missing 'ip' parameter.");
}

$mysqli = connect();
$stmt =  $mysqli->stmt_init();
$stmt->prepare('select mac, name, local_ip, date from my where gateway_ip = ? order by local_ip ') OR die("Invalid statement");

$stmt->bind_param("s", $gatewayIp);
$stmt->execute();

$stmt->bind_result($mac, $name, $localIp, $date);

$modules = array();
// Copy into an array to finish all DB related stuff before displaying the table.
while($row = $stmt->fetch()) {
  array_push($modules, array("date" => $date, "mac" => $mac, "name" => $name,
               "localeIp" => $localeIp));

}
$stmt->free_result();
$stmt->close();

?>
      <table>
        <?
        $count = count($module);
        
        for($i = 0; $i < $count; $i++) {
          $name = $module[$i]["name"];
          $mac = $module[$i]["mac"];
          $localeIp = $module[$i]["localeIp"];
          $date = $module[$i]["date"];
          
          echo "<tr><td>" + htmlspecialchars($name) + "</td></tr>"
        }
        ?>        
        
    </body>
</html>
        
        