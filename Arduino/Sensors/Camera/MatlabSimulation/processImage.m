function [out] = processImage(m, n, image)
    out = rgb2gray(image);
    for i = 1:m
        for j = 1:n
            if (out(i,j) < 128)
                out(i,j) = 0;
            else
                out(i,j) = 255;
            end
        end
    end
end