function globals() {
    return {
        send: (url, data, progress) => {
			return new Promise((resolve, reject) => {
				var ajax = new XMLHttpRequest();
				ajax.onreadystatechange = function () {
				    if(ajax.readyState == 4) {
				        if(ajax.status == 200){
							resolve(ajax.response);
						} else if(ajax.status == 401){
							reject("ERR_LOGIN_REQUIRED");
						} else {
							reject("ERR_UNKNOWN_STATUS");
						}
					}
				}
				ajax.onerror = function (err) {
					reject(err);
				}
				ajax.upload.addEventListener("progress", progress);
				ajax.open("POST", url, true);
				ajax.send(data);
			});
		},
		urlf: (url, args) => {
			var results = url;
			var length = Object.keys(args).length;
			if(length > 0) {
				results += "?";
			}
			for(var i  = 0; i < length; i++) {
				results += encodeURIComponent(Object.keys(args)[i]);
				results += "=";
				results += encodeURIComponent(Object.values(args)[i]);
				if(length - 1 != i) {
					results += "&";
				}
			}
			return results;
		}
    };
};

module.exports = globals;