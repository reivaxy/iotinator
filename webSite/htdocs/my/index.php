
<?

require('../../includes/getModules.php');

$modules = getList();

// If only one result, redirect browser to this ip.
// Otherwise display the list with links to each

if(count($modules) == 1) {
  $ip = $modules[0]["localIp"];
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
            $localIp = htmlspecialchars($modules[$i]["localIp"]);
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