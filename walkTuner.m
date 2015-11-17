function walkTuner(filename)
global threshold steps mx mn frames oldA A
% Load File
fileID = fopen(filename,'r');
dataArray = textscan(fileID,  '%f%f%f%f%f%[^\n\r]', 'Delimiter', ',', 'HeaderLines' ,1, 'ReturnOnError', false);
fclose(fileID);

% Organize File
X = dataArray{:, 1};
Y = dataArray{:, 2};
Z = dataArray{:, 3};
% A = dataArray{:, 4};
% H = dataArray{:, 5};
clear fileID dataArray; close all

% Define constants
dt = 0.1;
t = dt:dt:(dt*length(X)); t = t';

% Define variables
threshold = 75 + t*0;
steps = 0 + t*0;
mx = 0 + t*0; mn = 500 + t*0;
oldA = 0 + t*0;
A = t*0;

% void loop()
for i = 1:length(t)
    frames(i:end) = frames(i:end) + 1;
    
    % Obtain Value
    A(i:end) = getParameter(X(i),Y(i),Z(i));
    steps = steps + isStep(A(i),i);
    
    % Adapt threshold
    if A(i) > mx(i)
        mx(i:end) = A(i);
    end
    if A(i) < mn(i)
        mn(i:end) = A(i);
    end
    if mod(frames,1./dt) == 0
        threshold(i:end) = (mx(i)+mn(i))/2;
        mn(i:end) = 500; mx(i:end) = 0;
    end
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
plot(t, Z,'b'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('Z'); xlabel('Time, s')
linkaxes([a1 a2 a3],'x')

figure(2);
b1=subplot(3,1,1:2);
plot(t,A,'k',t,threshold,'r'), box off, grid on, set(gca,'XTickLabel',{})
ylabel('A^2')
b2=subplot(3,1,3);
plot(t,steps,'k'), box off, grid on, set(gca,'XTickLabel',{}),
ylabel('Steps'); xlabel('Time, s')
linkaxes([b1 b2],'x')

function A = getParameter(x,y,z)
A = x*x + y*y + z*z;

function isTrue = isStep(A, i)
global oldA threshold
if oldA(i) > A && A < threshold(i)
    isTrue = 1;
else
    isTrue = 0;
end
