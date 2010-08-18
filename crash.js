var RetBuf = require('./retbuf').RetBuf;

var retbuf = new RetBuf;
retbuf.getBufAsync(function (buf) {
   // should get 'buf' here, but node.js crashes
});

