function [num] = processNumber(number, nums_x, nums_y, num_m, num_n)
    x = sum(number, 1);
    y = sum(number, 2);
    dist_x = abs(sum(nums_x - x, 2))/num_m;
    dist_y = abs(sum(nums_y - y, 1))/num_n;
    if (min(dist_x)<=min(dist_y))
        num = find(dist_x == min(dist_x))-1;
    else
        num = find(dist_y == min(dist_y))-1;
    end
end