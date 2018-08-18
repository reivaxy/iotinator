      

<?
error_reporting(E_ALL);
ini_set("display_errors", 1);
ini_set('display_startup_errors', 1);


require('../../includes/utils.inc.php');

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
               "localeIp" => $localIp));

}
$stmt->free_result();
$stmt->close();
// If only one result, redirect browser to this ip.
// Otherwise display the list with links to each

if(count($modules) == 1) {
  $ip = $modules[0]["localeIp"];
  header("Location: http://$ip");  
  return;
}
?>
<!--  
      It's very unlikely that anyone will have 2 iotinator masters 
      So I'm not spending too much time on it :)
-->
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1"> 
    <link rel="stylesheet" href="css/my.css" />
  </head>
    <title>My Iotinator</title>
    <body>
        <?
        $count = count($modules);
        if($count == 0) {
          echo "<span class=\"error\">No module found for ip $gatewayIp</span><br/>";          
          echo '<span>Make sure you are connected to your local network.</span>';          
        } else {
?>
      <table>
        <tr>
          <th>Module</th>
          <th>Local IP</th>
          <th>Registration date</th>
        </tr>
<?        
          for($i = 0; $i < $count; $i++) {
            $name = $modules[$i]["name"];
            $mac = $modules[$i]["mac"];
            $localIp = htmlspecialchars($modules[$i]["localeIp"]);
            $date = $modules[$i]["date"];
            
            echo "<tr>";
            echo "<td>" . htmlspecialchars($name) . "</td>";
            echo "<td><a href=\"http://$localIp\">$localIp</a></td>";
            echo "<td>" . htmlspecialchars($date) . "</td>";
            echo "</tr>";
          }
          echo "</table>";
        }
        ?>        
        
    </body>
</html>