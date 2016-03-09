close gcf;
clear;

T1CoefNum = 64;
woRes = 6*16*16; %96;
wiRes = 6*16*16;
txtname = 'Res16g800.hgc.txt';
full = textread(txtname, '%f');
full = reshape(full, T1CoefNum, woRes)';
%contour(full);
%surfc(full(1:10,1:10));
    axis([1 wiRes 1 woRes]);
    P = zeros(woRes,wiRes);
    for i = 1: woRes
        for j = 1 : T1CoefNum
            P(i, full(i, j) + 1) = i;
        end
    end
    plot(1:wiRes, P, '.');
    xlabel('Coef Positions in Cubemap');
    ylabel('Coef Series')
    title(['Distribution of retained coefs positions']);
    fn = [txtname, '.jpg'];
    saveas(gcf, fn);
    close gcf;
clear;
