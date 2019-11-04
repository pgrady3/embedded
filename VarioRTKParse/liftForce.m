function f = liftForce(y, vx)

    clalpha = 0.6/5; %Cl per degree of alpha
    wingarea = 11; %meters squared
    density = 1.225; %density air

    x = y(1);
    vz = y(3);
    
    [windx, windz] = wind(x);
    
    rwind = sqrt((vx + windx)^2 + (vz + windz)^2);
    alpha = atan2d(-vz + windz, vx + windx);
    
    f = 0.5 * density * wingarea * (rwind^2) * clalpha * alpha;
end

