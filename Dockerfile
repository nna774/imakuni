FROM gcc:latest

WORKDIR /workspace

RUN wget https://gist.githubusercontent.com/nna774/9b06603599bd26986126af6fa50436f4/raw/1988ea75cbaa494ad1bc96e0d56188d15f04fd6a/imagediff.sh -O /usr/local/bin/imagediff && chmod +x /usr/local/bin/imagediff

COPY . .

RUN make
RUN make test
