# NoRCE extension for PHP

php-norce is a PHP extension that allows you to verify the digital signatures of files and code snippets before they are executed.


* Prevent the execution of unauthorized PHP files.
* Prevent the execution of unauthorized code snippets via both the ‘eval()’ function and the ‘php -r’ command line.
* Prevent the import of unauthorized PHP files.

## Installation

### Extension

Install dependencies

```bash
apt-get update
apt-get install build-essential libssl-dev php-dev
```

Clone repository

```bash
git clone git@github.com:otiekrk/php-norce.git
```

Build sign_files util
```bash
cd php-norce/util
make build
```

Create list of trusted scripts / snippets

```
php.txt
snippet.txt
```

Generate hash:
```bash
random_value=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32) \
&& hashed_value=$(echo -n $random_value | sha256sum | awk '{print $1}') \
&& echo $hashed_value
```

Create dirs for signatures (SHA256 hash)
```bash
mkdir /eebd76c4d1a5769900d88fb4cda108df1eccbc3493c0baf50ccf774161a7aa77 # For PHP files
mkdir /7f58ec50614aa6e94c1ef544acfe9e8b2cf99ae89c692b3dcdfbd4725a776b18 # For PHP snippets
```

Generate key and sign files
```bash
sign_files php.txt snippet.txt eebd76c4d1a5769900d88fb4cda108df1eccbc3493c0baf50ccf774161a7aa77 7f58ec50614aa6e94c1ef544acfe9e8b2cf99ae89c692b3dcdfbd4725a776b18
```

Public key export

* Move *.sign files to PHP signature dir and *.snip files to PHP snip dir.
```bash
mv ./*.sign /eebd76c4d1a5769900d88fb4cda108df1eccbc3493c0baf50ccf774161a7aa77 # For PHP files
mv ./*.snip /7f58ec50614aa6e94c1ef544acfe9e8b2cf99ae89c692b3dcdfbd4725a776b18 # For PHP snippets
```
* Move public key file (norce_key.h) to extension source dir.

```bash
mv ./norce_key.h ../extension/norce_key.h
```

Build extension
```bash
cd php-norce/extension
phpize
./configure --enable-norce
make
make install
mv ./modules/norce.so /your_folder/norce.so
```
then add this line to php.ini
```
extension=/your_folder/norce.so
```

## Example
Clone repository and run run-docker.sh.

```bash
git clone git@github.com:otiekrk/php-norce.git
cd php-norce
chmod +x ./run-docker.sh
./run-docker.sh
```

See installation example in Dockerfile.


## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

## License

MIT