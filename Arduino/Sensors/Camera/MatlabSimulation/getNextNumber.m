function [image, num] = getNextNumber(image, num_m, num_n)
    image = deleteRightMargin(image);
    [m, n] = size(image);
    num = image(1:num_m, n-num_n+1:n);
    image = deleteRightObject(image);
end