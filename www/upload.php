<?php

$uploadDir = '/var/www/html/';

function isValidPath(string $path): bool {
    return strpos($path, '..') === false;
}

if(isset($_FILES["file"]) && $_FILES["file"]["error"] === UPLOAD_ERR_OK) {
    $file = $_FILES['file'];
    if(isValidPath($file["name"])) {
        $targetFilePath = $uploadDir . $file["name"];
        if(move_uploaded_file($file["tmp_name"], $targetFilePath)) {
            $image = $uploadDir . $file['name'];
            echo "<script>alert('File uploaded successfully as " . basename($targetFilePath) . "');</script>";
        }
    }
}

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>PHP-NoRCE</title>
</head>
<body>
    <div>
        <form action="upload.php" method="post" enctype="multipart/form-data">
            <input type="file" name="file">
            <button type="submit">Upload</button>
        </form>
    </div>
</body>
</html>
