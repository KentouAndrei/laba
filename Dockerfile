FROM alpine AS build
RUN apk add --no-cache build-base make automake autoconf git pkgconfig glib-dev gtest-dev gtest cmake

WORKDIR /home/optima
RUN git clone --branch branchHTTPserver https://github.com/KentouAndrei/laba.git
WORKDIR /home/optima/laba

RUN autoconf
RUN ./configure
RUN cmake

FROM alpine
COPY --from=build /home/optima/laba/myprogram /usr/local/bin/myprogram
ENTRYPOINT ["/usr/local/bin/myprogram"]