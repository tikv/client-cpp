FROM centos:7.9.2009

RUN yum -y install centos-release-scl \
                   epel-release; \
                   yum -y install devtoolset-7-gcc \
                   devtoolset-7-gcc-c++ \
                   python3 \
                   make \
                   libtool \
                   openssl-devel \
                   yum clean all; \
                   rm -rf /var/cache/yum;

ENV PATH /opt/rh/devtoolset-7/root/bin/:$PATH
RUN pip3 install --upgrade pip --no-cache-dir && pip3 install --no-cache-dir cmake==3.26.4 -i https://pypi.tuna.tsinghua.edu.cn/simple

ENV RUSTUP_HOME=/usr/local/rustup CARGO_HOME=/usr/local/cargo
ENV PATH $CARGO_HOME/bin:$PATH
RUN mkdir -p "$CARGO_HOME" && mkdir -p "$RUSTUP_HOME" && \
    curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain 1.70.0 && \
    chmod -R a=rwX $CARGO_HOME
RUN cargo install cxxbridge-cmd --version 1.0.18

WORKDIR /client-cpp
