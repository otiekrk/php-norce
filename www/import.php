<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP-NoRCE</title>
</head>
<body>
<div>
    <?php

    include_once "./imports/trusted_script.php";

    $output = sprintf("Variable: %s", $variable ?? "NULL");

    echo $output;
    ?>
</div>
</body>
</html>