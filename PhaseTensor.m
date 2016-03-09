clear;
path =  'F:\Work\Wang05\PhaseTensor\PRT\Data\HG\';

DmtL = 384;
DmtV = 384;
DmtH = 19;
DmtLC = 4; %deb 8;
DmtVC = 3; %deb 8;
DmtHC = 2;%1,2,3,4 all will be tested
OrgDmt = [DmtL DmtV DmtH];
CmpDmt = [DmtLC DmtVC DmtHC];

iterMax = 30; %deb 50

%load brdf into tensor
BN0 = zeros(DmtH, DmtL * DmtV); %brdf
temp = zeros(DmtL * DmtV, 1);
for i = 1 : DmtH  %fetch brdfs
    bFile = sprintf('%sHG8g%03d.hg', path, -90 + 10 *(i-1));
    f = fopen(bFile, 'r');
    temp = fread(f, DmtL * DmtV, 'float32');    
    BN0(i,:) = temp';
    fclose(f);
end
clear temp;

Tsr0 = reshape(BN0', DmtL, DmtV, DmtH);
%don't mess up L and V!!
for i = 1 : DmtH
    Tsr0(:,:,i) = Tsr0(:,:,i)';
end

%init State
Tsr = Tsr0;
%Compress Tsr(20, 384, 384) and Save the core and basis, and Recover it to RecTsr
% 				U1 = rand(DmtL, DmtLC);
%               U2 = rand(DmtV, DmtVC);
%               U3 = rand(DmtH, DmtHC);
    U1 = ones(DmtL, DmtLC);
    U2 = ones(DmtV, DmtVC);
    U3 = ones(DmtH, DmtHC);

    for iter = 1 : iterMax
        %keep L, compress V,H:for
        %X2: compress V
        for i = 1 : DmtH
            for j = 1 : DmtL
                tmp = Tsr(j,:,i);
                tmp = reshape(tmp, 1, DmtV);
                Cmp2(j,:,i) = tmp * U2;
            end
        end
        % X3:compress H
        for i = 1 : DmtL
            for j = 1 : DmtVC
                tmp = Cmp2(i,j,:);
                tmp = reshape(tmp, 1, DmtH);
                Cmp23(i,j,:) = tmp * U3;
            end
        end
        % uf(L)
        Flt1 = zeros(DmtL,DmtVC * DmtHC);
        for i = 1 : DmtL
            for j = 1 : DmtVC
                for k = 1 : DmtHC
                    Flt1(i, (j-1)* DmtHC + k) =Cmp23(i,j,k);
                end
            end
        end

        %keep V, compress H,L:
        %X3: compress H
        for i = 1 : DmtL
            for j = 1 : DmtV
                tmp = Tsr(i,j,:);
                tmp = reshape(tmp, 1, DmtH);
                Cmp3(i,j,:) = tmp * U3;
            end
        end
        %X1: compress L
        for i = 1 : DmtV
            for j = 1 : DmtHC
                tmp = Cmp3(:,i,j);
                tmp = reshape(tmp, 1, DmtL);
                Cmp31(:,i,j) = tmp * U1;
            end
        end
        %uf(V)
        Flt2 = zeros(DmtV, DmtHC * DmtLC);
        for i = 1 : DmtV
            for j = 1 : DmtHC
                for k = 1 : DmtLC
                    Flt2(i, (j-1) * DmtLC + k)=Cmp31(k,i,j);
                end
            end
        end

        %keep H, compress L,V:
        %compress L
        for i = 1 : DmtV
            for j = 1 : DmtH
                tmp = Tsr(:,i,j);
                tmp = reshape(tmp, 1, DmtL);
                Cmp1(:,i,j) = tmp * U1;
            end
        end
        %compress V
        for i = 1 : DmtH
            for j = 1 : DmtLC
                tmp = Cmp1(j,:,i);
                tmp = reshape(tmp, 1, DmtV);
                Cmp12(j,:,i) = tmp * U2;
            end
        end
        %uf(H)
        Flt3 = zeros(DmtH,DmtLC * DmtVC);
        for i = 1 : DmtH
            for j = 1 : DmtLC
                for k = 1 : DmtVC
                    Flt3(i, (j-1) * DmtVC + k)=Cmp12(j,k,i);
                end
            end
        end

% 					[u_1, s] = pcacov(Flt1 * Flt1');
% 					[u_2, s] = pcacov(Flt2 * Flt2');
% 					[u_3, s] = pcacov(Flt3 * Flt3');
% 					
% 					U1 = u_1(:, 1:DmtLC);
% 					U2 = u_2(:, 1:DmtVC);
% 					U3 = u_3(:, 1:DmtHC);

        [U1, s, v] = svds(Flt1, DmtLC);
        [U2, s, v] = svds(Flt2, DmtVC);
        [U3, s, v] = svds(Flt3, DmtHC);
        
        %calc out B
        % X1: compress L
        Tsr1 = zeros(DmtLC, DmtV, DmtH);
        for i = 1 : DmtV
            for j = 1 : DmtH
                tmp = Tsr(:,i,j);
                tmp = reshape(tmp, 1, DmtL);
                Tsr1(:,i,j) = tmp * U1;
            end
        end
        % X2: compress V
        Tsr2 = zeros(DmtLC, DmtVC, DmtH);
        for i = 1 : DmtLC
            for j = 1 : DmtH
                tmp = Tsr1(i, :, j);
                tmp = reshape(tmp, 1, DmtV);
                Tsr2(i,:,j) = tmp * U2;
            end
        end
        % X3: compress H
        Tsr3 = zeros(DmtLC, DmtVC, DmtHC);
        for i = 1 : DmtLC
            for j = 1 : DmtVC
                tmp = Tsr2(i, j, :);
                tmp = reshape(tmp, 1, DmtH);
                Tsr3(i,j,:) = tmp * U3;
            end
        end

        iter
        if (iter > 1)
           OldFlt = reshape(CoreOld, 1, DmtLC * DmtVC * DmtHC);
           NewFlt = reshape(Tsr3,    1, DmtLC * DmtVC * DmtHC);
           dif = OldFlt * OldFlt' - NewFlt * NewFlt'
           %dif = trace(CoreOld * CoreOld') - trace(Tsr3 * Tsr3')
        end
        if (exist([path, '\stoptensor']))
            delete ([path, '\stoptensor']);
            break;
        end

       CoreOld = Tsr3;
    end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    for i = 1 : DmtHC
        Tsr3tmp(:,:,i) = Tsr3(:,:,i)';
    end

    Tsr3 = reshape(Tsr3tmp, DmtLC * DmtVC, DmtHC)';
    Tsr3 = reshape(Tsr3, DmtLC, DmtVC,  DmtHC);

    % dump core
    fnm = sprintf('%sCore', path);
    fid = fopen(fnm, 'w');
    fwrite(fid, Tsr3, 'float32');
    fclose(fid);

    % dump U1: 
    for j = 1: DmtLC
        fnm = sprintf('%sRank_%04d_Total_%04d_Index_%04d',path, 0, DmtLC, j - 1);
        fid = fopen(fnm, 'w');
        fwrite(fid, U1(:, j), 'float32');
        fclose(fid);
    end
    % dump U2:
    for j = 1: DmtVC
        fnm = sprintf('%sRank_%04d_Total_%04d_Index_%04d',path, 1, DmtVC, j - 1);
        fid = fopen(fnm, 'w');
        fwrite(fid, U2(:, j), 'float32');
        fclose(fid);
    end
    % dump U3:
    for j = 1: DmtHC
        fnm = sprintf('%sRank_%04d_Total_%04d_Index_%04d',path, 2, DmtHC, j - 1);
        fid = fopen(fnm, 'w');
        fwrite(fid, U3(:, j), 'float32');
        fclose(fid);
    end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%recover core to RecTsr
    filename = [path,'core'];
    fid = fopen(filename, 'r');
    [core, count] = fread(fid, DmtLC * DmtVC * DmtHC, 'float32');
    fclose(fid)
    core = reshape(core, DmtHC, DmtLC * DmtVC)';
    core = reshape(core, DmtVC, DmtLC, DmtHC);
    for i = 1 : DmtHC
        coretmp(:,:,i) = core(:,:,i)';
    end
    core = coretmp;

    %'finish core'

    LgtMat = zeros(DmtLC,DmtL);
    for i = 1 : DmtLC
        filename = sprintf('%sRank_0000_Total_%04d_Index_%04d', path, DmtLC, i - 1);
        fid = fopen(filename,'r');
        [tmp, count] = fread(fid, DmtL, 'float32');
        LgtMat(i,:) = double(tmp)';
        fclose(fid);
    end

    ViwMat = zeros(DmtVC,DmtV);
    for i = 1 : DmtVC
        filename = sprintf('%sRank_0001_Total_%04d_Index_%04d', path, DmtVC, i - 1);
        fid = fopen(filename,'r');
        [tmp, count] = fread(fid, DmtV, 'float32');
        ViwMat(i,:) = double(tmp)';
        fclose(fid);
    end

    HMat = zeros(DmtHC,DmtH);
    for i = 1 : DmtHC
        filename = sprintf('%sRank_0002_Total_%04d_Index_%04d', path, DmtHC, i - 1);
        fid = fopen(filename,'r');
        [tmp, count] = fread(fid, DmtH, 'float32');
        HMat(i,:) = double(tmp)';
        fclose(fid);
    end

    %'finish light view h'

    brdfcore = zeros(DmtL,DmtV,DmtHC);
    for i = 1 : DmtVC
        for j = 1 : DmtHC
            brdfcore(:,i,j) = (core(:,i,j)' * LgtMat)';
        end
    end
    % 
    % for i = 1 : DmtVC
    %     for j = 1 : DmtHC
    %         filename = sprintf('sRct_Lgt_%04d_%04d', path, i-1, j-1);
    %         fid = fopen(filename, 'w');
    %         RctLgt = single(brdfcore(:,i,j));
    %         fwrite(fid,RctLgt,'float32');
    %         fclose(fid);
    %     end
    % end

    %'finish Rct_Lgt_'

    for i = 1 : DmtL
        for j = 1 : DmtHC
            brdfcore(i,:,j) = (brdfcore(i,1:DmtVC,j) * ViwMat)';
        end
    end
    % 
    % for i = 1 : DmtHC
    %     filename = sprintf('%sBRDF_Basis_%04d', path, i-1);
    %     fid = fopen(filename, 'w');
    %     RctLgt = single(brdfcore(:,:,i));
    %     fwrite(fid,RctLgt,'float32');
    %     fclose(fid);
    % end

    for i = 1 : DmtH
        brdf = zeros(DmtL, DmtV);
        for j = 1 : DmtHC
            brdf = brdf + brdfcore(:,:,j) * HMat(j,i); 
        end
        %brdf = single(brdf);
%                    RecTsr(i,:) = reshape(brdf, 1, DmtL * DmtV) ;
        %deb
        RecTsr(i,:) = reshape(brdf', 1, DmtL * DmtV) ;
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%Recovered Tsr0 to RecTsr
    %write each brdf's normed rmse
    errfilename = sprintf('%sTensor%dRMSE%dX%dX%d.txt', path, DmtL, DmtLC, DmtVC, DmtHC);
    err_fid = fopen(errfilename, 'at');

    %write each brdf's rmse, relrmse
    for i = 1: DmtH
        sub = RecTsr(i, :) - BN0(i, :);
        sigmaDelta2 = sum(sum(sub .* sub));
        sigmaBN0i2 = sum(sum(BN0(i, :) .* BN0(i, :)));
        fprintf(err_fid, '%f\t%f\n', sqrt(sigmaDelta2),  sqrt(sigmaDelta2 / sigmaBN0i2));
    end
    subTsr = RecTsr - BN0;
    sigmaDeltaT2 = sum(sum(sum(sub .* sub)));
    sigmaBN02 = sum(sum(sum(BN0 .* BN0)));
    fprintf(err_fid, '%f\t%f\n', sqrt(sigmaDeltaT2), sqrt(sigmaDeltaT2 /sigmaBN02));
    fclose(err_fid);

%             %plot E
%             figure(1);
%             plot(E);
%             xlabel('BRDF#')
%             ylabel('E')
%             titleE = ['Tsr, alpha', num2str(alpha),', Iter', int2str(errorIter), ', std(E)',num2str(stdE),', E'];
%             title(titleE);
%             %axis([1 100 0.05 0.5]);
%             hold on;
%             X = 1 : DmtH;
%             plot(X, meanE, '-.g');
%             text(DmtH, meanE, ['mean=',num2str(meanE)],...
%                 'HorizontalAlignment','right');
%             grid on;
%             %dump
%             saveas(1, [path, titleE,'.bmp']);
%             
%             for i = 1: DmtH
%                 filename = sprintf('%sB%04dIter%04d.bdf', path, i - 1, errorIter);
%                 fid = fopen(filename, 'w');
%                 fwrite(fid, RecTsr(i, :) * NormsBN(i), 'float32');
%                 fclose(fid);
%             end
%         end

close all;
clear;
'finish'

        
        
        
        
        
        
        
        
        
%         
% E = zeros(1, DmtH); %RMSE
% sumDeltaSquare = 0;
% for i = 1 : DmtH
%     %read Orig.
%     filename = sprintf('D:\\share\\Work\\TensorwithCos\\%04d.bdf', i-1);
%     fid = fopen(filename, 'r');
%     [brdfOrig, count] = fread(fid, DmtL * DmtV, 'float32');
%     fclose(fid);
%     brdfOrig = reshape(brdfOrig, DmtV, DmtL);
% 
%     %RMSE
%     deltaBi = double(brdf) - double(brdfOrig);
%     sumDeltaSquareBi = sum(sum(deltaBi .* deltaBi));
%     E(i) = sqrt(sumDeltaSquareBi / (DmtV * DmtL));
%     fprintf(err_fid, '%f\n', E(i));
%     
%     %sum RMSE cumulative
%     sumDeltaSquare = sumDeltaSquare + sumDeltaSquareBi;
% end
%     volumeRMSE = sqrt(sumDeltaSquare / (DmtV * DmtL * DmtH));
%     fprintf(err_fid, 'volumeRMSE%f\n', volumeRMSE);
% 
% fclose(err_fid);
% clear;
% 'finish !'
