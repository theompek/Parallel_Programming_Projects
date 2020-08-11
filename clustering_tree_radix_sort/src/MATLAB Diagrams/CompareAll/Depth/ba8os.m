function [ output_args ] = ba8os(megistoba8os,Cmegistoba8os16,Pmegistoba8os16,Omegistoba8os16 )
subplot(221);
plot(megistoba8os(:,1),megistoba8os(:,2),'.-.k');
title('HashCode')
xlabel('The depth of the three')
ylabel('Time(Seconds)')

hold on
plot(megistoba8os(:,1),Cmegistoba8os16(:,2),'g');
plot(megistoba8os(:,1),Pmegistoba8os16(:,2),'k');
plot(megistoba8os(:,1),Omegistoba8os16(:,2)),'r';

legend('grammikh','Cilk','OpenMP','Pthreads')
hold off

subplot(222)
plot(megistoba8os(:,1),megistoba8os(:,3),'.-.k');
title('MortonCode')
xlabel('The depth of the three')
ylabel('Time(Seconds)')

hold on
plot(megistoba8os(:,1),Cmegistoba8os16(:,3),'g');
plot(megistoba8os(:,1),Pmegistoba8os16(:,3),'k');
plot(megistoba8os(:,1),Omegistoba8os16(:,3)),'r';

legend('grammikh','Cilk','OpenMP','Pthreads')
hold off


subplot(223)
plot(megistoba8os(:,1),megistoba8os(:,4),'.-.k');
title('RadixSort')
xlabel('The depth of the three')
ylabel('Time(Seconds)')

hold on
plot(megistoba8os(:,1),Cmegistoba8os16(:,4),'g');
plot(megistoba8os(:,1),Pmegistoba8os16(:,4),'k');
plot(megistoba8os(:,1),Omegistoba8os16(:,4)),'r';

legend('grammikh','Cilk','OpenMP','Pthreads')
hold off

subplot(224)
plot(megistoba8os(:,1),megistoba8os(:,5),'.-.k');
title('Rarrangment')
xlabel('The depth of the three')
ylabel('Time(Seconds)')


hold on
plot(megistoba8os(:,1),Cmegistoba8os16(:,5),'g');
plot(megistoba8os(:,1),Pmegistoba8os16(:,5),'k');
plot(megistoba8os(:,1),Omegistoba8os16(:,5)),'r';

legend('grammikh','Cilk','OpenMP','Pthreads')
hold off
end

