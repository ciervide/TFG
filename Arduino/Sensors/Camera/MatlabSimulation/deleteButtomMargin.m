function [out] = deleteButtomMargin(image)
    out = flip(deleteTopMargin(flip(image,1)),1);
end