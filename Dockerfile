FROM python

RUN apt update &&\
    apt install -y build-essential nano wget nano \
    libgl1-mesa-dev v4l-utils

RUN pip3 install cmake

COPY requirements.txt /tmp/requirements.txt
RUN pip3 install -r /tmp/requirements.txt