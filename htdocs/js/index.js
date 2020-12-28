import $ from 'jquery';
import 'angular-material';
import 'angular-material/angular-material.css';
import '../css/fonts.css';
import '../css/index.css';
import '../css/login.css';
import '../css/dialog.css';
import '../css/static.css';

var factory = require('./factory.js');
var directives = require('./directive.js');

var imageExt = ["jpg", "png", "jpeg", "bmp", "h264", "gif", "svg"];
var videoExt = ["mov", "mp4", "avi", "wmv", "3gp", "mpg", "mpeg"];
var audioExt = ["wav", "ogg", "mp3", "m4a", "aac"];

var chipdrive = angular.module('chipdrive', ['ngMaterial']);
chipdrive.directive('onChange', directives.onChange);
chipdrive.factory('globals', factory);

chipdrive.controller("chipdrivectrl", ['$scope', '$mdDialog', 'globals', ($scope, $mdDialog, globals) => {
	$scope.onerror = (e) => {
		if(e == "ERR_LOGIN_REQUIRED") {
			$scope.login();
		} else {
			$scope.showSpinner = false;
			$scope.showError = true;
			$scope.errorMsg = e;
			$scope.$apply();
		}
	}

	$scope.list = async (id) => {
		id == null ? id = $scope.root : id;
		$scope.items = [];
		$scope.showEmptyPlaceholder = false;
		$scope.showProgress = false;
		$scope.showError = false;
		$scope.showSpinner = true;
		globals.send(globals.urlf("/api/v1/drive/list", {
			"folderid": id
		}), null, null)
		.then((response) => {
			var items = JSON.parse(response).data.list;
			$scope.showSpinner = false;
			$scope.items = items;
			$scope.currentFolder = id;
			if(items.length == 0) {
				$scope.showEmptyPlaceholder = true;
			}
			$scope.$apply();
		}).catch((e) => {
			$scope.onerror(e);
		});
	}

	$scope.upload = async (evt) => {
		$scope.items = [];
		$scope.showProgress = true;

		var totalLength = 0;
		var sent = 0;

		var files = evt.target.files;
		for(var i = 0; i < files.length; i++) {
			totalLength += files[i].size;
		}
		for(var i = 0; i < files.length; i++) {
			try {
				await globals.send(globals.urlf("/api/v1/drive/upload", {
					"folderid": $scope.currentFolder, 
					"name": files[i].name
				}), files[i], (event) => {
					var percent = ((sent + event.loaded) / totalLength) * 100;
					$scope.progress = percent.toFixed(2);
					$scope.$apply();
				});
				sent += files[i].size;
			} catch(e) {
				$scope.onerror(e);
			}
		}

		$scope.showProgress = false;
		$scope.list($scope.currentFolder);
	}

	$scope.createFolder = () => {
		var nameDialog = $mdDialog.prompt()
		.title('How would you like to name your new folder?')
		.textContent('')
		.placeholder('NewFolder1')
		.required(true)
		.ok('Okay')
		.cancel('Cancel');

		$mdDialog.show(nameDialog)
		.then(function (name) {
			$scope.items = [];
			$scope.showSpinner = true;
			globals.send(globals.urlf("/api/v1/drive/folder", {
				"folderid": $scope.currentFolder, 
				"name": name
			}), null, null)
			.then(() => {
				$scope.showSpinner = false;
				$scope.list($scope.currentFolder);
			}).catch((e) => {
				$scope.onerror(e);
			});
		}).catch((e) => {
			// Cancel
		});
	}

	$scope.renameItem = (item) => {
		var nameDialog = $mdDialog.prompt()
		.title('Rename Item')
		.textContent('')
		.placeholder('')
		.required(true)
		.ok('Okay')
		.cancel('Cancel');

		$mdDialog.show(nameDialog)
		.then(function (name) {
			$scope.items = [];
			$scope.showSpinner = true;
			globals.send(globals.urlf("/api/v1/drive/rename", {
				"itemid": item.id, 
				"name": name
			}), null, null)
			.then(() => {
				$scope.showSpinner = false;
				$scope.list($scope.currentFolder);
			}).catch((e) => {
				$scope.onerror(e);
			});
		}).catch((e) => {
			// Cancel
		});
	}

	$scope.deleteItem = (item) => {
		var nameDialog = $mdDialog.confirm()
		.title('Delete Item?')
		.textContent('')
		.ok('Okay')
		.cancel('Cancel');

		$mdDialog.show(nameDialog)
		.then(function () {
			$scope.items = [];
			$scope.showSpinner = true;
			globals.send(globals.urlf("/api/v1/drive/delete", {
				"itemid": item.id
			}), null, null)
			.then(() => {
				$scope.showSpinner = false;
				$scope.list($scope.currentFolder);
			}).catch((e) => {
				$scope.onerror(e);
			});
		}).catch((e) => {
			// Cancel
		});
	}

	$scope.downloadItem = (item) => {
		var src = globals.urlf("/api/v1/drive/stream", {
			"id": item.id
		});
		var a = document.createElement("a");
		a.style.display = "none";
		a.style.height = "0px";
		a.href = src;
		a.download = item.name;
		document.body.appendChild(a);
		a.click();
		document.body.removeChild(a);
		$mdDialog.cancel();
	}

	$scope.chooseUpload = () => {
		$('#upload-hidden').trigger('click');
	}

	$scope.openItem = (x, ev) => {
		switch(x.type) {
			case 1: {
				$mdDialog.show({
					controller: OpenItemController,
					locals:{
						item: x
					},
					templateUrl: "template/dialog.html",
					parent: angular.element(document.body),
					targetEvent: ev,
					clickOutsideToClose: true,
					fullscreen: $scope.customFullscreen
				});
			}
			break;
			case 2: {
				$scope.list(x.id);
			}
			break;
		}
	}

	$scope.login = () => {
		$mdDialog.show({
			controller: LoginController,
			locals:{
				init: $scope.init
			},
			templateUrl: "template/login.html",
			parent: angular.element(document.body),
			clickOutsideToClose: false,
			fullscreen: $scope.customFullscreen
		});
	}

	$scope.init = (driveid) => {
		globals.send(globals.urlf("/api/v1/drive/config", {
			"driveid": driveid
		}), null, null)
		.then((response) => {
			var config = JSON.parse(response).data;
			$scope.root = config.root;
			$scope.list(null);
		}).catch((e) => {
			$scope.onerror(e);
		});
	}

	$scope.init("my_drive");

	$scope.navItem = "my_drive";
}]);

function LoginController($scope, $mdDialog, globals, init) {
	$scope.showLogin = true;
	$scope.login = () => {
		$scope.errorText = "Logging In...";
		globals.send(globals.urlf("/api/v1/login", {
			"username": $scope.username, 
			"password": $scope.password,
			"time"    : Math.floor(new Date().getTime() / 1000)
		}), null, null)
		.then((e) => {
			var json = JSON.parse(e);
			if(json.success == true) {
				$mdDialog.cancel();
				init('my_drive');
			} else {
				$scope.errorText = json.reason;
				$scope.$apply();
			}
		}).catch((e) => {
			$scope.errorText = e;
			$scope.$apply();
		});
	}
}

LoginController.$inject = ['$scope', '$mdDialog', 'globals', "init"];

function OpenItemController($scope, $mdDialog, globals, item) {
	$scope.name = item.name;
	$scope.src = globals.urlf("/api/v1/drive/stream", {
		"id": item.id
	});
	
	var ext = item.name.substr(item.name.lastIndexOf('.') + 1).toLowerCase();
	if(imageExt.indexOf(ext) >= 0) {
		$scope.type = 'image';
	} else if(audioExt.indexOf(ext) >= 0) {
		$scope.type = 'audio';
	} else if(videoExt.indexOf(ext) >= 0) {
		$scope.type = 'video';
	} else {
		$scope.type = 'unknown';
	}

	$scope.cancel = () => {
		$mdDialog.cancel();
    };
}

OpenItemController.$inject = ['$scope', '$mdDialog', 'globals', 'item'];