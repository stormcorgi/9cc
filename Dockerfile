FROM ubuntu:latest
RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt install -y gcc make git binutils libc6-dev gdb sudo
## change uid to the number which you use in host machine.
RUN adduser --disabled-password --gecos '' --uid 1002 user
RUN echo 'user ALL=(root) NOPASSWD:all' > /etc/sudoers.d/user
USER user
WORKDIR /home/user
