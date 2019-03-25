FROM alpine:latest

# Update
RUN apk add --update nodejs nodejs-npm

RUN apk add --no-cache make gcc g++ python linux-headers udev

# Install app dependencies
COPY package.json /src/package.json
RUN npm install serialport --build-from-source=serialport
RUN cd /src; npm install

# Bundle app source
COPY . /src

EXPOSE  3000
CMD ["node", "/src/server.js"]
