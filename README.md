Miguel Angel Lopez Sanchez fc65675
Alejandro Dominguez fc64447
Bruno Felisberto fc32435

limitations:
  Quando o processo principal cria uma transação, ela é armazenada no buffer main_wallet, depois a wallet correspondente lê a transação e libera a posição do buffer em que estava. Em seguida, essa wallet tenta escrever no wallet_server, mas se não houver espaço disponível, ela fica em espera. Portanto, o alarme não captura essa transação, pois ela está armazenada em uma variável local da wallet que está aguardando para escrever no wallet_server.

como executar:
  make clean
  make
  ./bin/SOchain args.txt settings.txt
