#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ec.h>

#define KEY_LEN 32

struct keypair {
    unsigned char privateKey[KEY_LEN];
    unsigned char publicKey[KEY_LEN];
};

void generate(struct keypair *key) {
    EVP_PKEY_CTX *ctx_edwards = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);

    if (ctx_edwards == NULL) {
        return;
    }
    if (EVP_PKEY_keygen_init(ctx_edwards) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }
    EVP_PKEY *edwardsKey = NULL;
    if (EVP_PKEY_keygen(ctx_edwards, &edwardsKey) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }
    size_t kPrivE = 32;
    size_t kPubE = 32;
    if (1 != EVP_PKEY_get_raw_private_key(edwardsKey, key->privateKey, &kPrivE)) {
        ERR_print_errors_fp(stderr);
    }
    if (1 != EVP_PKEY_get_raw_public_key(edwardsKey, key->publicKey, &kPubE)) {
        ERR_print_errors_fp(stderr);
    }
    EVP_PKEY_CTX_free(ctx_edwards);
    EVP_PKEY_free(edwardsKey);
}

void writeConfiguration(struct keypair *key, const char *signatureDir, const char *snippetDir) {
    FILE *f = fopen("norce_key.h", "w+");
    if (f == NULL) {
        puts("fopen() failed");
        exit(1);
    }
    const char *buff_unsigned_char = "extern unsigned char norce_key[32] = \"";
    const char *buff_char = "extern char ";
    const char *buff_signature = "norce_signature_dir[65] = \"";
    const char *buff_snippet = "norce_snippet_dir[65] = \"";
    const char *buff_endline = "\";\n";

    fwrite(buff_unsigned_char, sizeof(char), strlen(buff_unsigned_char), f);
    for (int i = 0; i < KEY_LEN; i++) {
        char hex_buff[5];
        sprintf(hex_buff, "\\x%02x", key->publicKey[i]);
        fwrite(hex_buff, sizeof(char), strlen(hex_buff), f);
    }
    fwrite(buff_endline, sizeof(char), strlen(buff_endline), f);

    fwrite(buff_char, sizeof(char), strlen(buff_char), f);
    fwrite(buff_signature, sizeof(char), strlen(buff_signature), f);
    fwrite(signatureDir, sizeof(char), strlen(signatureDir), f);
    fwrite(buff_endline, sizeof(char), strlen(buff_endline), f);

    fwrite(buff_char, sizeof(char), strlen(buff_char), f);
    fwrite(buff_snippet, sizeof(char), strlen(buff_snippet), f);
    fwrite(snippetDir, sizeof(char), strlen(snippetDir), f);
    fwrite(buff_endline, sizeof(char), strlen(buff_endline), f);

    fclose(f);
}

char *readPHPFile(const char *filename, long *numbytes) {
    FILE *infile = fopen(filename, "r");
    char *buffer = NULL;

    if (infile != NULL) {
        fseek(infile, 0L, SEEK_END);
        *numbytes = ftell(infile);
        fseek(infile, 0L, SEEK_SET);
        buffer = (char *) malloc(*numbytes);
        fread(buffer, sizeof(char), *numbytes, infile);
    }

    fclose(infile);

    return buffer;
}

void hashBytes(char *data, size_t dataLength, unsigned char *hashValue, unsigned int *hashLength) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    if (mdctx == NULL) {
        ERR_print_errors_fp(stderr);
        return;
    }

    if (1 != EVP_DigestInit_ex2(mdctx, EVP_MD_fetch(NULL, "sha256", NULL), NULL)) {
        ERR_print_errors_fp(stderr);
        return;
    }

    if (!EVP_DigestUpdate(mdctx, data, dataLength)) {
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        return;
    }
    if (!EVP_DigestFinal_ex(mdctx, hashValue, hashLength)) {
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        return;
    }

    EVP_MD_CTX_free(mdctx);
}

char * signFile(char *filename, unsigned char *_key, unsigned char *signature, size_t *signLen, size_t *dataLen) {
    long numbytes;
    unsigned char *buffer =  (unsigned char*)readPHPFile(filename, &numbytes);

    if (buffer == NULL) {
        return NULL;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_PKEY *key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, _key, KEY_LEN);
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new(key, NULL);

    if (mdctx == NULL) {
        ERR_print_errors_fp(stderr);
        free(buffer);
        return NULL;
    }
    if (1 != EVP_DigestSignInit_ex(mdctx, &pctx, NULL, NULL, NULL, key, NULL)) {
        ERR_print_errors_fp(stderr);
        free(buffer);
        return NULL;
    }

    if (1 != EVP_DigestSign(mdctx, signature, signLen, buffer, numbytes)) {
        ERR_print_errors_fp(stderr);
        free(buffer);
        return NULL;
    }
    EVP_PKEY_free(key);
    EVP_MD_CTX_free(mdctx);

    *dataLen = numbytes;

    return (char*)buffer;
}

void signFiles(struct keypair *k, const char *filename, int signSnippets) {
    FILE *f_list = fopen(filename, "r");
    if (f_list == NULL) {
        printf("Failed to open %s\n", filename);
        exit(1);
    }

    int maxLineLength = 400;
    char lineBuffer[maxLineLength];
    while (fgets(lineBuffer, maxLineLength, f_list) != NULL) {
        size_t nl_char_pos = strlen(lineBuffer);
        if (lineBuffer[nl_char_pos - 1] == '\n') {
            lineBuffer[nl_char_pos - 1] = '\x00';
        }
        printf("Signing %s.\n", lineBuffer);
        unsigned char signature[64];
        size_t signLen;
        size_t dataLen = 0;
        char *fileData = signFile(lineBuffer, k->privateKey, signature, &signLen, &dataLen);
        if (fileData == NULL) {
            memset(lineBuffer, 0, maxLineLength);
            free(fileData);
            continue;
        }
        unsigned char hash_buff[32];
        char hash_str_buff[65];
        char filename_buff[70];
        unsigned int hash_buff_len;

        if (signSnippets == 1) {
            hashBytes(fileData, dataLen, hash_buff, &hash_buff_len);
        } else {
            hashBytes(lineBuffer, strlen(lineBuffer), hash_buff, &hash_buff_len);
        }

        free(fileData);

        for (int i = 0; i < hash_buff_len; i++) {
            sprintf(&hash_str_buff[i * 2], "%02x", hash_buff[i]);
        }

        if (signSnippets == 1) {
            sprintf(filename_buff, "%s.snip", hash_str_buff);
        } else {
            sprintf(filename_buff, "%s.sign", hash_str_buff);
        }

        printf("Save sign file as: %s\n", filename_buff);

        FILE *f = fopen(filename_buff, "w+");
        if (f != NULL) {
            fwrite(signature, sizeof(char), signLen, f);
        }
        fclose(f);
        memset(lineBuffer, 0, maxLineLength);
    }
    fclose(f_list);
}

int main(int argc, char **argv) {
    if (argc != 5) {
        puts("Usage: sign_files <php list> <snippet list> <signature dir> <snippet dir>\n\n");
        puts("Example: sign_files php.txt snippet.txt signatures snippet-signatures\n");
        return 1;
    }
    struct keypair k;
    generate(&k);
    signFiles(&k, argv[1], 0);
    signFiles(&k, argv[2], 1);
    writeConfiguration(&k, argv[3], argv[4]);

    return 0;
}
