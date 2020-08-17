function [out] = deleteTopMargin(image)
    [m, n] = size(image);
    i = find(sum(image, 2)~=0, 1);
    out = image(i:m,:);
end