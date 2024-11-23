FROM alpine 
WORKDIR /home
COPY ./myprogram .
RUN apk add libstdc++
RUN apk add libc6-compat
ENTRYPOINT ["./myprogram"]
