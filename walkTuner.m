function walkTuner(filename)
% Load File
fileID = fopen([filename '.txt'],'r');
dataArray = textscan(fileID,  '%f%f%f%f%f%f%[^\n\r]', 'Delimiter', ',', 'HeaderLines' ,1, 'ReturnOnError', false);
fclose(fileID);

clear global;
global millis RATE RSTEP TSTEP threshold steps lastStep frameStart thresholdReset ox oy oz sx sy sz op oldParam stepsSinceReset X Y Z

% Organize File
X = dataArray{:, 1};
Y = dataArray{:, 2};
Z = dataArray{:, 3};
A = dataArray{:, 4};
T = dataArray{:, 5};
S = dataArray{:, 6};
clear fileID dataArray; close all

RATE = 100;
TSTEP = 500;
RSTEP = 4;
BRATE = 300;
WEIGHT = 30;

millis = (RATE:RATE:RATE*length(A))';

lastStep = 0;
frameStart = 0;
thresholdReset = 0;
ox = X(1);
oy = Y(1);
oz = Z(1);
sx = 0;
sy = 0;
sz = 0;
op = [0 0 0 0];
oldParam = 0;
stepsSinceReset = 0;

threshold = 0 * millis;
steps = 0 * millis;
param = 0 * millis;

for i = 1:length(millis)
    
   if mod(i,BRATE) == 0
       ox = X(i);
       oy = Y(i);
       oz = Z(i);
   end
   param(i:end) = getParameter(i);
   steps(i:end) = steps(i) + 2*isStep(param(i), i);
   
   threshold(i:end) = ((WEIGHT-1)*threshold(i) + param(i))/WEIGHT;%1000*(threshold(i)*(millis(i)-RATE)/1000 + param(i)/10)/millis(i);
end

% Plot
figure(1)
a1=subplot(6,1,1:2);
plot(millis, X,'r'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('X')
a2=subplot(6,1,3:4);
plot(millis, Y,'g'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('Y')
a3=subplot(6,1,5:6);
plot(millis, Z,'b'), box off, grid on
ylabel('Z'); xlabel('Time, ms')
linkaxes([a1 a2 a3],'x'), xlim([0 6e4]);

figure(2);
b1=subplot(3,1,1:2);
plot(millis,param,'k',millis,threshold,'r',millis,A,'k--',millis,T,'r--'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('A^2')
b2=subplot(3,1,3);
plot(millis,steps,'k',millis,S,'k--'), box off, grid on
ylabel('Steps'); xlabel('Time, ms')
linkaxes([b1 b2],'x'), xlim([0 6e4]);

function bool = isStep(param,i)
global millis RATE RSTEP TSTEP threshold steps lastStep frameStart thresholdReset ox oy oz sx sy sz op oldParam stepsSinceReset X Y Z %#ok<NUSED>

if param <= threshold(i) && oldParam > threshold(i)
   flag = 1;
   if (millis(i) - lastStep < TSTEP)
       sx = 0; sy = 0; sz = 0;
       ox = X(i); oy = Y(i); oz = Z(i);
       stepsSinceReset = 0;
       threshold(i:end) = 0;
       thresholdReset = millis(i);
   else
       stepsSinceReset = stepsSinceReset + 1;
   end
   lastStep = millis(i);
else
    flag = 0;
end

oldParam = param;
if stepsSinceReset < RSTEP
    bool = 0;
elseif stepsSinceReset == RSTEP
    bool = flag * stepsSinceReset;
else
    bool = flag;
end

function int = getParameter(i)
global millis RATE RSTEP TSTEP threshold steps lastStep frameStart thresholdReset ox oy oz sx sy sz op oldParam stepsSinceReset X Y Z %#ok<NUSED>

op(1) = op(2); op(2) = op(3); op(3) = op(4);

sx = sx + X(i) - ox; sy = sy + Y(i) - oy; sz = sz + Z(i) - oz;

ox = X(i); oy = Y(i); oz = Z(i);

op(4) = sy;
if op(4) < sx
    op(4) = sx;
end
if op(4) < sz
    op(4) = sz;
end

int = (op(1) + op(2) + op(3) + op(4))/4;

