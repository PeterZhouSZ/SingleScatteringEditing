clear;
itplN = 10;
T1CoefNum = 256;
n6k = 6144;
full = textread('HGcIdxOverlap.txt', '%f');
for v = 100:110

    t = full(((v - 1) * itplN * T1CoefNum + 1) : v * itplN * T1CoefNum);
    t = reshape(t, T1CoefNum, itplN)';
    P = zeros(itplN, n6k);
    axis([1 n6k 1 itplN]);
    for i = 1: itplN
        for j = 1 : T1CoefNum
            P(i, t(i, j) + 1) = i;
        end
        plot(1:n6k, P(i,:), '.');
        hold all;
    end
    xlabel('Coef Positions in Cubemap');
    ylabel('Coef Series')
    title(['Distribution of retained coefs positions of vert', num2str(v-1)]);
    fn = ['v', num2str(v),'.jpg'];
    saveas(gcf, fn);
    %close gcf;
    clf;
    
end
clear;
