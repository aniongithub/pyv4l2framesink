FROM python

RUN apt update &&\
    apt install -y build-essential nano wget nano

RUN pip3 install cmake