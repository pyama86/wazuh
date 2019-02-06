FROM ubuntu:latest
RUN apt-get -qq update && \
    apt-get install -qq gcc make debhelper dh-make clang git curl devscripts
