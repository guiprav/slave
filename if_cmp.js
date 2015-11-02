'use strict';

let comparators = {};

module.exports = function(a, cmp, b, options) {
    if(comparators[cmp](a, b)) {
        return options.fn(this);
    }
    else {
        return options.inverse(this);
    }
};

comparators.eq = function(a, b) {
    return a === b;
};
