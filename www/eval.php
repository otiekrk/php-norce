<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP-NoRCE</title>
</head>
<body>
<div>
    <?php
    eval("echo \"Trusted eval\".PHP_EOL;");
    ?>
</div>
<div>
    <?php
    eval("echo \"Untrusted eval\".PHP_EOL;");
    ?>
</div>
</body>
</html>