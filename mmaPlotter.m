fileID = fopen('test4.txt','r');

dataArray = textscan(fileID,  '%f%f%f%[^\n\r]', 'Delimiter', ',', 'HeaderLines' ,1, 'ReturnOnError', false);
fclose(fileID); X = dataArray{:, 1}; Y = dataArray{:, 2}; Z = dataArray{:, 3};
clear fileID dataArray; close all

T = 0.1:0.1:(0.1*length(X));
A = sqrt(X.^2+Y.^2+Z.^2); A = (A-1).*(A>1);
%figure(); plot(T,X,'r-',T,Y,'g-',T,Z,'b-');
figure(); plot(T, A,'k-');

% Averager
ma = @(n) [2/n repmat(1/n, [1 n-1]) 2/n];
A2 = conv(A,ma(3)); 
%figure(); plot(A2,'k-');

% Compound
M = [mode(abs(X)), mode(abs(Y)), mode(abs(Z))];
[~,i] = sort(M);
K = [1 0.5 0.25]; K = K(i);
Xi = X*K(1); Yi = Y*K(2); Zi = Z*K(3);
%figure(); plot(T,Xi+Yi+Zi);

% Count Steps
THRESH = 0.5;
STEPS = 2*sum(diff(A>THRESH)>0)
