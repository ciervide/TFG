function [out] = deleteRightObject(image)
    [m, n] = size(image);
    i = n-find(sum(flip(image,2), 1)==0, 1)+1;
    out = image(:,1:i);
end