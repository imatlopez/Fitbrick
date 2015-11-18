function walkTuner(filename)

% Global variables
global RATE TSTEP RSTEP threshold steps lastStep frameStart thresholdReset ox oy oz sx sy sz op t oldParam stepsSinceReset %#ok<NUSED>

% Load File
fileID = fopen([filename '.txt'],'r');
dataArray = textscan(fileID,  '%f%f%f%f%f%f%[^\n\r]', 'Delimiter', ',', 'HeaderLines' ,1, 'ReturnOnError', false);
fclose(fileID);

% Organize File
X = dataArray{:, 1};
Y = dataArray{:, 2};
Z = dataArray{:, 3};
A = 100*dataArray{:, 4};
T = 100*dataArray{:, 5};
S = 2*dataArray{:, 6};
clear fileID dataArray; close all

% Define constants
TSTEP = 500;
RSTEP = 4;
RATE = 100;
dt = RATE/1000;
t = dt:dt:(dt*length(X)); t = t';

% Define variables
threshold = 0 + t*0;
steps = 0 + t*0;
oldParam = 0;
param = t*0;

% Define parameters
ox = 0; oy = 0; oz = 0; sx = 0; sy = 0; sz = 0;
stepsSinceReset = 0;
op = [0 0 0 0];

% void loop()
for i = 1:length(t)
    
    % Obtain Value
    param(i:end) = getParameter(100*X(i),100*Y(i),100*Z(i));
    steps(i:end) = steps(i:end) + 2*isStep(param(i),i);
    
    % Adapt threshold
    threshold(i:end) = (threshold(i)*(t(i)-dt) + param(i)/10)/t(i);
    
end

% Plot
figure(1)
a1=subplot(6,1,1:2);
plot(t, X,'r'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('X')
a2=subplot(6,1,3:4);
plot(t, Y,'g'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('Y')
a3=subplot(6,1,5:6);
plot(t, Z,'b'), box off, grid on
ylabel('Z'); xlabel('Time, s')
linkaxes([a1 a2 a3],'x')

figure(2);
b1=subplot(3,1,1:2);
plot(t,param,'k',t,threshold,'r',t,A,'k--',t,T,'r--'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('A^2')
b2=subplot(3,1,3);
plot(t,steps,'k',t,S,'k--'), box off, grid on
ylabel('Steps'); xlabel('Time, s')
linkaxes([b1 b2],'x')

function param = getParameter(x,y,z)
global op ox oy oz sx sy sz

% Cycle through moving averager
op(1) = op(2); op(2) = op(3); op(3) = op(4);

% Update sum parameters
sx = sx + (x - ox);
sy = sy + (y - oy);
sz = sz + (z - oz);

% Update old values
ox = x; oy = y; oz = z;

% Determine parameter
op(4) = sy;
if op(4) < sx
    op(4) = sx;
end
if op(4) < sz
    op(4) = sz;
end

% Return moving averager
param = (op(1) + op(2) + op(3) + op(4))/4;


function isTrue = isStep(param, i)
global RATE TSTEP RSTEP threshold steps lastStep frameStart thresholdReset ox oy oz sx sy sz op t oldParam stepsSinceReset %#ok<NUSED>
% Detect step
if oldParam > threshold(i) && param < threshold(i)
    flag = 1;
    % Verify integrity of step
    if t(i) - lastStep < TSTEP
        % Reset thresholding
        sx = 0; sy = 0; sz = 0;
        stepsSinceReset = 0;
        thresholdReset = t(i);
    else 
        stepsSinceReset = stepsSinceReset + 1;
    end
    lastStep = t(i);
else
    flag = 0;
end
oldParam = param;

% Determine if data is reliable to publish
if stepsSinceReset < RSTEP
    isTrue = 0;
elseif stepsSinceReset == RSTEP
    isTrue = stepsSinceReset*flag;
else
    isTrue = flag;
end
