clear;

res = 8; %then should repeat everything to 32/res times
n1 = 6*res*res;
n2 = 32; 
path = 'F:\Work\Wang05\PhaseTensor\PRT\Data\HG\';
para0 = 'HG8g';
for g = -90: 10: 90
    para = [para0 num2str(g)];
    fn = [path, para, '.hg'];
    f = fopen(fn, 'rb');
    mat = fread(f, n1 * n1, 'float32');
    mat = reshape(mat, n1, n1);
    fclose(f);
    imshow(mat);

    %[u s v] = svds(mat, n2);
    [u s v] = svd(mat);

    %test
    mat1 = u * s * v';
    eps = sum(sum(mat1 - mat))
    'test';

    s1 = sqrt(diag(s));
    for i = 1 : n1
        u(:, i) = u(:, i) .* s1(i);
        v(:, i) = v(:, i) .* s1(i);
    end 
    %test
    %mat1 = u(:, 1:n1) * v(:, 1:n1)';
    mat1 = u(:, 1:n2) * v(:, 1:n2)'; 
    eps = sum(sum(mat1 - mat))
    'test';

    fn = [path, para, 't', num2str(n2), '.hgG'];
    f = fopen(fn, 'wb');
    for i = 1 : n2
        fwrite(f, u(:, i), 'float32'); %one col
    end
    fclose(f);

    fn = [path, para, 't', num2str(n2), '.hgH'];
    f = fopen(fn, 'wb');
    for i = 1 : n2
        fwrite(f, v(:, i), 'float32');
    end
    fclose(f);

    
    %RelRMSE
    errfilename = [path, 'RelRMSE', para, '.txt'];
    err_fid = fopen(errfilename, 'wt');
    for recoverNum = 1: 24
        %recoverNum = n1;
        mat1 = u(:, 1:recoverNum) * v(:, 1:recoverNum)'; % n1*n2 * n2*n2 * n2*n1 = n1*n1    
        sub = mat - mat1;
        RelRMSE = sqrt(sum(sum(sub .* sub)) / (sum(sum(mat .* mat))));
        fprintf(err_fid, '%f\n', RelRMSE);
        %write
        %figure(1);
        %imshow(mat1);
        %imgNm = [path, para, 't', num2str(n2), '.jpg'];
        %saveas(1, imgNm);
    end
    fclose(err_fid);
end    
'finished'

%     %rms
%     errfilename = [path, 'relRMS', para, '.txt'];
%     err_fid = fopen(errfilename, 'wt');
%     for recoverNum = 1: 24
%         mat1 = u(:, 1:recoverNum) * s(1:recoverNum, 1:recoverNum) * v(:, 1:recoverNum)'; % n1*n2 * n2*n2 * n2*n1 = n1*n1    
%         errSum = 0;
%         matSum = 0;
%         for i = 1 : n1
%             err = mat1(i, :) - mat(i, :);
%             err = err * err';
%             errSum = errSum + sum(err);
%             matSum = matSum + sum(mat(i, :) * mat(i, :)');
%         end
%         RMS = sqrt(errSum / matSum);
%     
%         fprintf(err_fid, '%f\n', RMS);
%     
%         %write
%         %figure(1);
%         %imshow(mat1);
%         %imgNm = [path, para, 't', num2str(n2), '.jpg'];
%         %saveas(1, imgNm);
%     end
%     fclose(err_fid);


