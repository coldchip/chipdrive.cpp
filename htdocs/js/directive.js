function onChange() {
	return {
		restrict: 'A',
		link: function (scope, element, attrs) {
			var onChangeHandler = scope.$eval(attrs.onChange);
			element.on('change', onChangeHandler);
			element.on('$destroy', function() {
				element.off();
			});

		}
	};
};

module.exports = { onChange };