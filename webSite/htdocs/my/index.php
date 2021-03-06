
<?

require('../includes/getModules.php');

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
      So I'm not spending too much time on this output :)
-->
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1"> 
    <link rel="stylesheet" href="css/my.css" />
    <script>
      function deleteModule(name, mac) {
        if(confirm(`Confirm deletion of module '${name}' ?`)) {
          const req = new XMLHttpRequest();
          req.open('GET', '/my/delete.php?mac=' + mac, false); 
          req.send(null);
          var id = `_${mac}`;
          var tr = document.getElementById(id);
          if(tr) {
            tr.parentNode.removeChild(tr);
          }
        }
      }
    </script>
  </head>
  <title>My Iotinator</title>
  <body>
<?
    $count = count($modules);
    $ip = $_SERVER['REMOTE_ADDR'];
    if($count == 0) {
      echo "<span class=\"error\">No iotinator master module found on your network $ip</span><br/>";          
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
          
          echo "<tr id=\"_". htmlspecialchars($mac) . "\">";
          echo "<td class=\"name\">" . htmlspecialchars($name) . "</td>";
          echo "<td class=\"link\"><a href=\"http://$localIp\">$localIp</a></td>";
          echo "<td class=\"date\">" . htmlspecialchars($date) . "</td>";
          echo "<td class=\"bin\"><span title=\"Delete\" onClick=\"deleteModule('" . 
                  htmlspecialchars($name) . "','" . htmlspecialchars($mac) . "')\" class=\"icon icon-bin\"/></td>";
          echo "</tr>";
        }
        echo "</table>";
      }
      ?>        
      
  </body>
</html>
