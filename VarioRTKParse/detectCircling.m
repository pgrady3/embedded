function [isCircling, heading] = detectCircling(N, E)
%detectCircling Outputs a binary vector if the aircraft is circling
%   N, E in meters
%   Assumes points are 0.1 sec apart
    
    pos = [E, N];

    isCirclingRaw = zeros(size(N));
    heading = zeros(size(N));
    LOOK_BACK = 70;
    LOOK_FWD = 70;
    HEADING_LF = 20;
    HEADING_LB = 20;
    
    for i = (LOOK_BACK + 1):(length(N) - LOOK_FWD)
        cur = pos(i, :);
        fwd =  pos(i + LOOK_FWD, :);
        back = pos(i - LOOK_BACK, :);
        
        df = fwd - cur;
        db = cur - back;
        dHead = pos(i + HEADING_LF, :) - pos(i - HEADING_LB, :);
        
        
        heading(i) = atan2(dHead(2), dHead(1)) * 180 / pi;
        
        thetaFwd =  atan2(df(2), df(1));
        thetaBack = atan2(db(2), db(1));
        dTheta = abs(angdiff(thetaFwd, thetaBack)) * 180 / pi;
        
        if (dTheta > 90) && (norm(df) > LOOK_FWD) %ensure forward speed
            isCirclingRaw(i) = 1;
        end
    end
    
    kernel = ones(50, 1);
    
    isCircling = conv(isCirclingRaw, kernel, 'same');
    isCircling(isCircling > 0.5) = 1;
    
    %isCircling = isCirclingRaw;
    
%     figure;
%     plot(isCirclingRaw); hold on;
%     plot(isCircling);
end

