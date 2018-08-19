<?
header("Content-Type: application/json");
require('../../includes/getModules.php');

$modules = getList();

echo json_encode($modules);

