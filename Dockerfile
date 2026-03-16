FROM ubuntu:latest

LABEL authors="otiekrk"

RUN useradd -m norce

RUN apt-get update \
    && apt-get install -y build-essential libssl-dev php php-dev \
    && apt-get -y autoremove \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN mkdir extension && mkdir sign-utility && mkdir build

COPY ./www /var/www/html
RUN chown -R norce:norce /var/www/html

WORKDIR /sign-utility

COPY ./util .
RUN make build  \
    && random_value_sign=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32) \
    && random_value_snippet=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32) \
    && hashed_value_sign=$(echo -n $random_value_sign | sha256sum | awk '{print $1}') \
    && hashed_value_snippet=$(echo -n $random_value_snippet | sha256sum | awk '{print $1}') \
    && mkdir /$hashed_value_sign && mkdir /$hashed_value_snippet \
    && ./sign_files php.txt snippet.txt $hashed_value_sign $hashed_value_snippet \
	&& mv ./norce_key.h /build/norce_key.h \
	&& mv ./*.sign /$hashed_value_sign/ \
	&& mv ./*.snip /$hashed_value_snippet/ \
    && chown -R norce:norce /$hashed_value_sign && chown -R norce:norce /$hashed_value_snippet

WORKDIR /build

COPY ./extension .

RUN phpize \
    && ./configure --enable-norce \
    && make \
    && make install\
    && random_value=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32) \
    && hashed_value=$(echo -n $random_value | sha256sum | awk '{print $1}') \
    && mkdir /$hashed_value \
    && mv ./modules/norce.so /$hashed_value/norce.so \
    && chown -R norce:norce /$hashed_value \
    && echo "extension=/$hashed_value/norce.so" | tee -a /etc/php/8.3/cli/php.ini /etc/php/8.3/apache2/php.ini

WORKDIR /var/www/html

RUN rm -rf /build && rm -rf /sign-utility && rm -rf /extension

USER norce

ENTRYPOINT ["php", "-S", "0.0.0.0:8000"]