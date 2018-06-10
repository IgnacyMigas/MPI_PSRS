FROM debian
MAINTAINER Masahiro Nakao <masahiro.nakao@riken.jp>
WORKDIR /home/xmp

RUN apt-get -y update  && \
    apt-get -y upgrade && \
    apt-get install -y git && \
    apt-get install -y --no-install-recommends flex gcc gfortran g++ openjdk-8-jdk \
            libopenmpi-dev openmpi-bin libxml2-dev byacc make perl bzip2 wget ssh less vim emacs && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
RUN useradd xmp -s /bin/bash
ADD http://omni-compiler.org/download/stable/omnicompiler-1.2.3.tar.bz2 /root
RUN cd /root && \
    tar xfj omnicompiler-1.2.3.tar.bz2 && \
    cd omnicompiler-1.2.3 && \
    ./configure && \
    make && \
    make install
ADD http://xcalablemp.org/download/lecture/beginner/2.globalview.tgz /home/xmp
ADD http://xcalablemp.org/download/lecture/beginner/3.localview.tgz /home/xmp
RUN cd /home/xmp && \
    tar xfz 2.globalview.tgz && \
    tar xfz 3.localview.tgz
RUN echo 'export PATH=/usr/local/bin:$PATH\nalias l="ls"' >> /home/xmp/.bashrc
RUN chown xmp:xmp /home/xmp 2.globalview.tgz 3.localview.tgz /home/xmp/.bashrc
