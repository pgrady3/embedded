function [wx,wz] = wind(x)
    
    if x > 100
        wx = 1;
    else
        wx = 0;
    end

    %wx = 1 * 0.5 * sin(x * 2 * pi / 100);

    if x > 200
        wz = 1;
    else
        wz = 0;
    end
end

