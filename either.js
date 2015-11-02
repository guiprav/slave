'use strict';

module.exports = function() {
    return [].find.call(arguments, function(value) {
        return !!value;
    });
};
