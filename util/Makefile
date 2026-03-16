clean:
	rm ./*.sign
	rm ./sign_files
	rm ./norce_key.h
build:
	gcc sign_files.c -o sign_files -lssl -lcrypto
