#include <iostream>
using namespace std;
#include <Windows.h>
#include <iomanip>
#include <string>
#include <fstream>
#include <conio.h>
#include <locale>


struct InstructionType;
struct ReservationStation;
struct Instruction;
struct LoadBuffer;
struct StoreBuffer;
struct InstructionStatus;
struct Tomasulo;

/* A estrutura InstructionStatus armazena informações sobre quando uma instrução foi emitida, começou sua execução, concluiu sua execução e escreveu de volta. */
struct InstructionStatus
{
    short issue;            // Ciclo em que a instrução foi emitida
    short executionStart;   // Ciclo em que a execução da instrução começou
    short executionComplete; // Ciclo em que a execução da instrução foi concluída
    short writeBack;        // Ciclo em que os resultados da instrução são escritos de volta ao registrador

    short executionCyclesRemaining; // Ciclos restantes para a conclusão da execução da instrução

    InstructionStatus()
    {
        issue = executionStart = executionComplete = writeBack = executionCyclesRemaining = -1;
    }
};
/* A estrutura InstructionType define os tipos de instrução suportados. */
struct InstructionType
{
public:
    static string const MULTD;
    static string const SUBD;
    static string const ADDD;
    static string const DIVD;
    static string const LOAD;
    static string const BNE;
    static string const STORE;
};

/* Definindo os tipos de instrução */
string const InstructionType::MULTD = "MUL";
string const InstructionType::SUBD = "SUB";
string const InstructionType::ADDD = "ADD";
string const InstructionType::DIVD = "DIV";
string const InstructionType::LOAD = "LOAD";
string const InstructionType::BNE = "BNE";
string const InstructionType::STORE = "STORE";

/* As estruturas ReservationStationType e LoadStoreBufferType definem tipos de estações de reserva e buffers de carga/armazenamento. */
struct ReservationStationType
{
    static string const ADDD;
    static string const MULTD;
};

string const ReservationStationType::ADDD = "ADD";
string const ReservationStationType::MULTD = "MUL";

struct LoadStoreBufferType
{
    static string const LOAD;
    static string const STORE;
};

string const LoadStoreBufferType::LOAD = "LOAD";
string const LoadStoreBufferType::STORE = "STORE";

/* A estrutura Instruction representa uma instrucao com seus campos relevantes. */
struct Instruction
{
    string instructionType;   // Tipo da instrucaoo
    string Rd;               // Registrador de destino
    string Rs;               // Registrador de origem 1
    string Rt;               // Registrador de origem 2
    int immediateOffset;     // Usado apenas para instrucoees do tipo I

    InstructionStatus instructionStatus; // Status da instrucaoo (emissao, execucao, etc.)

    Instruction()
    {
        instructionType = Rd = Rs = Rt = "";
        immediateOffset = -1;
    }
};

/* A estrutura RegisterStatus mantem informacoes sobre o status dos registradores. */
struct RegisterStatus
{
    string registerName; // Nome do registrador
    string writingUnit;  // Unidade de escrita que esta escrevendo no registrador
};

/* A estrutura ReservationStation representa uma estacao de reserva usada para instrucoes de ponto flutuante. */
struct ReservationStation
{
    string name;           // Nome da estacao de reserva
    bool isBusy;           // Indica se a estacao de reserva esta ocupada
    string instructionType; // Tipo de instrucao na estacao de reserva
    string Vj;             // Valor Vj (fonte de dados)
    string Vk;             // Valor Vk (fonte de dados)
    string Qj;             // Status Qj (indicando dependencia)
    string Qk;             // Status Qk (indicando dependencia)

    Instruction *instruction; // Ponteiro para a instrucao atualmente em execucao na estacao de reserva

// Construtor da classe ReservationStation
ReservationStation()
{
	// Inicializa a estação de reserva como não ocupada
	isBusy = false;
	// Inicializa a instrução como nula
	instruction = nullptr;
}
};

/* A estrutura LoadStoreBuffer e usada para instrucoes de carga e armazenamento. */
struct LoadStoreBuffer
{
    string name;    // Nome do buffer de carga/armazenamento
    bool isBusy;    // Indica se o buffer esta ocupado
    string address; // Endereco de memoria
    string fu;      // Unidade funcional

    Instruction *instruction; // Ponteiro para a instrucao atualmente em execucao no buffer
// Construtor da classe LoadStoreBuffer
LoadStoreBuffer()
{
	// Inicializa o nome como uma string vazia
	name = "";
	// Inicializa a buffer de carga/armazenamento como não ocupada
	isBusy = false;
	// Inicializa o endereço como uma string vazia
	address = "";
	// Inicializa a instrução como nula
	instruction = nullptr;
	// Inicializa a unidade funcional como uma string vazia
	fu = "";
}
};

/* A funcao gotoxy e usada para definir a posicao do cursor no console. */

void gotoxy(short, short);

struct Tomasulo
{

	string reason; // razão
	int currentCycleNumber; // número do ciclo atual
	LoadStoreBuffer *loadBuffers; // buffers de carga
	LoadStoreBuffer *storeBuffers; // buffers de armazenamento
	ReservationStation *addSubReservationStations; // estações de reserva para adição e subtração
	ReservationStation *multDivReservationStations; // estações de reserva para multiplicação e divisão
	RegisterStatus *registerStatus; // status do registro

	int totalLoadBuffers; // total de buffers de carga
	int totalStoreBuffers; // total de buffers de armazenamento
	int totalAddSubReservationStations; // total de estações de reserva para adição e subtração
	int totalMultDivReservationStations; // total de estações de reserva para multiplicação e divisão

	int totalRegisters; // total de registros
	Instruction* instructions; // instruções
	int totalInstructions; // total de instruções

	//quantos ciclos cada tipo de instrução leva no estágio de execução.
	int totalLoadStoreCycles;
	int totalAddSubCycles;
	int totalMultCylcles;
	int totalDivCycles;



	// A função loadDataFromFile é responsável por carregar dados de um arquivo de texto.
	void loadDataFromFile(string fileName)
	{
		// Se o nome do arquivo não for fornecido, ele usará "source.txt" como padrão.
		if (fileName == "")
		{
			fileName = "source.txt";
		}

		// Abre o arquivo para leitura.
		ifstream read;
		read.open(fileName);

		// Se o arquivo não puder ser aberto, ele exibirá uma mensagem de erro e encerrará o programa.
		if (read.is_open() == false)
		{
			cout << "File could not open!" << endl;
			system("pause");
			exit(EXIT_FAILURE);
		}

		string data;

		// Ignora tudo até o primeiro '#'.
		while (read.peek() != '#')
			read.ignore();

		// Ignora a primeira linha após o '#'.
		getline(read, data);

		// Inicializa variáveis booleanas para verificar se todas as informações necessárias foram lidas do arquivo.
		bool loadDone = false,
			storeDone = false,
			addSubDone = false,
			multDivDone = false;

		bool exit_ = false;

		// Lê informações sobre o número de buffers/estações de reserva.
		// Loop que executa 4 vezes
		for (int i = 0; i < 4; i++)
		{
			// Lê um valor do arquivo de entrada e armazena em 'data'
			read >> data;

			// Se 'data' for igual a "Add_Sub_Reservation_Stations"
			if (_strcmpi(data.c_str(), "Add_Sub_Reservation_Stations") == 0)
			{
				// Define 'addSubDone' como verdadeiro
				addSubDone = true;

				// Lê o próximo valor do arquivo de entrada e armazena em 'totalAddSubReservationStations'
				read >> this->totalAddSubReservationStations;
			}
			// Se 'data' for igual a "Mul_Div_Reservation_Stations"
			else if (_strcmpi(data.c_str(), "Mul_Div_Reservation_Stations") == 0)
			{
				// Define 'multDivDone' como verdadeiro
				multDivDone = true;

				// Lê o próximo valor do arquivo de entrada e armazena em 'totalMultDivReservationStations'
				read >> this->totalMultDivReservationStations;
			}
			// Se 'data' for igual a "Load_Buffers"
			else if (_strcmpi(data.c_str(), "Load_Buffers") == 0)
			{
				// Define 'loadDone' como verdadeiro
				loadDone = true;

				// Lê o próximo valor do arquivo de entrada e armazena em 'totalLoadBuffers'
				read >> this->totalLoadBuffers;
			}
			// Se 'data' for igual a "Store_Buffers"
			else if (_strcmpi(data.c_str(), "Store_Buffers") == 0)
			{
				// Define 'storeDone' como verdadeiro
				storeDone = true;

				// Lê o próximo valor do arquivo de entrada e armazena em 'totalStoreBuffers'
				read >> this->totalStoreBuffers;
			}
		}

		// Se alguma das informações necessárias estiver faltando, ele exibirá uma mensagem de erro e encerrará o programa.
		// Se a operação de armazenamento não estiver concluída
		if (!storeDone)
		{
			// Imprime uma mensagem informando que ainda faltam buffers de armazenamento
			cout << "-> Informação sobre o número de buffers de armazenamento faltando." << endl;
			// Define a variável 'exit_' como verdadeira
			exit_ = true;
		}
		// Se a operação de carga não estiver concluída
		else if (!loadDone)
		{
			// Imprime uma mensagem informando que ainda faltam buffers de carga
			cout << "-> Informação sobre o número de buffers de carga faltando." << endl;
			// Define a variável 'exit_' como verdadeira
			exit_ = true;
		}
		// Se as operações de multiplicação/divisão não estiverem concluídas
		else if (!multDivDone)
		{
			// Imprime uma mensagem informando que ainda faltam estações de reserva de multiplicação/divisão
			cout << "-> Informação sobre o número de estações de reserva de multiplicação/divisão faltando" << endl;
			// Define a variável 'exit_' como verdadeira
			exit_ = true;
		}
		// Se as operações de adição/subtração não estiverem concluídas
		else if (!addSubDone)
		{
			// Imprime uma mensagem informando que ainda faltam estações de reserva de adição/subtração
			cout << "-> Informação sobre o número de estações de reserva de adição/subtração faltando." << endl;
			// Define a variável 'exit_' como verdadeira
			exit_ = true;
		}

		// Continua a ler o arquivo para obter mais informações, como o número de ciclos de instrução.

		// lê uma string, compara com várias opções possíveis, armazena as informações correspondentes e verifica se todas as informações necessárias foram lidas.

		//carregar informação para o ciclo de instruções
			bool multCyclesDone = false,
				 divCyclesDones = false,
				 addSubCyclesDone = false,
				 loadStoreCyclesDone = false;

			while (read.peek() != '#')
				read.ignore();

		getline(read,data);
		//cout << data << endl;
		// Loop que executa 4 vezes
		for (int i = 0; i < 4; i++)
		{
			// Lê um valor do arquivo de entrada e armazena em 'data'
			read >> data;

			// Se 'data' for igual a "Add_Sub_Cycles"
			if (_strcmpi(data.c_str(), "Add_Sub_Cycles") == 0)
			{
				// Lê o próximo valor do arquivo de entrada e armazena em 'totalAddSubCycles'
				read >> this->totalAddSubCycles;

				// Define 'addSubCyclesDone' como verdadeiro
				addSubCyclesDone = true;
			}
			// Se 'data' for igual a "Mul_Cycles"
			else if (_strcmpi(data.c_str(), "Mul_Cycles") == 0)
			{
				// Lê o próximo valor do arquivo de entrada e armazena em 'totalMultCylcles'
				read >> this->totalMultCylcles;

				// Define 'multCyclesDone' como verdadeiro
				multCyclesDone = true;
			}
			// Se 'data' for igual a "Load_Store_Cycles"
			else if (_strcmpi(data.c_str(), "Load_Store_Cycles") == 0)
			{
				// Lê o próximo valor do arquivo de entrada e armazena em 'totalLoadStoreCycles'
				read >> this->totalLoadStoreCycles;

				// Define 'loadStoreCyclesDone' como verdadeiro
				loadStoreCyclesDone = true;
			}
			// Se 'data' for igual a "Div_Cycles"
			else if (_strcmpi(data.c_str(), "Div_Cycles") == 0)
			{
				// Lê o próximo valor do arquivo de entrada e armazena em 'totalDivCycles'
				read >> this->totalDivCycles;

				// Define 'divCyclesDones' como verdadeiro
				divCyclesDones = true;
			}
		}

				// Se as operações de adição/subtração não estiverem concluídas
				if (!addSubCyclesDone)
				{
					// Imprime uma mensagem informando que ainda faltam ciclos de adição/subtração
					cout << "-> Informação sobre o número de ciclos de adição/subtração faltando.";
					// Define a variável 'exit_' como verdadeira
					exit_ = true;
				}

				// Se as operações de multiplicação não estiverem concluídas
				if (!multCyclesDone)
				{
					// Imprime uma mensagem informando que ainda faltam ciclos de multiplicação
					cout << "-> Informação sobre o número de ciclos de multiplicação faltando." << endl;
					// Define a variável 'exit_' como verdadeira
					exit_ = true;
				}
				// Se as operações de divisão não estiverem concluídas
				else if (!divCyclesDones)
				{
					// Imprime uma mensagem informando que ainda faltam ciclos de divisão
					cout << "-> Informação sobre o número de ciclos de divisão faltando." << endl;
					// Define a variável 'exit_' como verdadeira
					exit_ = true;
				}
				// Se as operações de carga/armazenamento não estiverem concluídas
				else if (!loadStoreCyclesDone)
				{
					// Imprime uma mensagem informando que ainda faltam ciclos de carga/armazenamento
					cout << "-> Informação sobre o número de ciclos de carga/armazenamento faltando." << endl;
					// Define a variável 'exit_' como verdadeira
					exit_ = true;
				}

				// Se a variável 'exit_' for verdadeira
				if (exit_ == true)
				{
					// Pausa a execução do programa
					system("pause");
					// Termina a execução do programa com um status de falha
					exit(EXIT_FAILURE);
				}

				// Alocando memória para os buffers de carga baseado no total de buffers de carga
				this->loadBuffers = new LoadStoreBuffer[this->totalLoadBuffers];

				// Loop para inicializar cada buffer de carga
				for (int i = 0; i < totalLoadBuffers; i++)
				{
					char num[10]; // Variável para armazenar o número do buffer como uma string
					_itoa_s(i, num, 10); // Convertendo o número do buffer para string

					// Definindo o nome do buffer de carga com o tipo de buffer (LOAD) e o número do buffer
					this->loadBuffers[i].name = LoadStoreBufferType::LOAD;
					this->loadBuffers[i].name.append(num);
				}

				// Alocando memória para os buffers de armazenamento baseado no total de buffers de armazenamento
				this->storeBuffers = new LoadStoreBuffer[this->totalStoreBuffers];

				// Loop para inicializar cada buffer de armazenamento
				for (int i = 0; i < totalStoreBuffers; i++)
				{
					char num[10]; // Variável para armazenar o número do buffer como uma string
					_itoa_s(i, num, 10); // Convertendo o número do buffer para string

					// Definindo o nome do buffer de armazenamento com o tipo de buffer (STORE) e o número do buffer
					this->storeBuffers[i].name = LoadStoreBufferType::STORE;
					this->storeBuffers[i].name.append(num);
				}

				// Alocando memória para as estações de reserva de adição/subtração baseado no total dessas estações
				this->addSubReservationStations = new ReservationStation[this->totalAddSubReservationStations];

				// Loop para inicializar cada estação de reserva de adição/subtração
				for (int i = 0; i < this->totalAddSubReservationStations; i++)
				{
					char num[10]; // Variável para armazenar o número da estação como uma string
					_itoa_s(i, num, 10); // Convertendo o número da estação para string

					// Definindo o nome da estação de reserva com o tipo de estação (ADDD) e o número da estação
					this->addSubReservationStations[i].name = ReservationStationType::ADDD;
					this->addSubReservationStations[i].name.append(num);
				}

				// Alocando memória para as estações de reserva de multiplicação/divisão baseado no total dessas estações
				this->multDivReservationStations = new ReservationStation[this->totalMultDivReservationStations];

				// Loop para inicializar cada estação de reserva de multiplicação/divisão
				for (int i = 0; i < this->totalMultDivReservationStations; i++)
				{
					char num[10]; // Variável para armazenar o número da estação como uma string
					_itoa_s(i, num, 10); // Convertendo o número da estação para string

					// Definindo o nome da estação de reserva com o tipo de estação (MULTD) e o número da estação
					this->multDivReservationStations[i].name = ReservationStationType::MULTD;
					this->multDivReservationStations[i].name.append(num);
				}


				// Enquanto o próximo caractere no arquivo não for '#', ignore-o
				while (read.peek() != '#')
					read.ignore();

				// Lendo informações sobre o número de registros
				getline(read, data); // Lê e descarta uma linha
				read >> data; // Lê e descarta a próxima entrada
				read >> this->totalRegisters; // Lê o número total de registros

				// Alocando memória para o status dos registros baseado no total de registros
				this->registerStatus = new RegisterStatus[totalRegisters];

				// Loop para inicializar cada registro
				for (int i = 0; i < totalRegisters; i++)
				{
					this->registerStatus[i].registerName = "F"; // Define o nome do registro como 'F'
					char num[10]; // Variável para armazenar o número do registro como uma string
					_itoa_s(i, num, 10); // Convertendo o número do registro para string
					this->registerStatus[i].registerName.append(num); // Adiciona o número do registro ao nome do registro
				}

				// Lendo instruções
				while (read.peek() != '#') // Enquanto o próximo caractere no arquivo não for '#', ignore-o
					read.ignore();

				getline(read, data); // Ignora a primeira linha

				// Lê o número total de instruções
				read >> this->totalInstructions;

				// Alocando memória para as instruções baseado no total de instruções
				this->instructions = new Instruction[this->totalInstructions];

				// Loop para ler todas as instruções
				for (int i = 0; i < totalInstructions; i++)
				{
					string instruction;
					read >> instruction; // Lê a instrução do arquivo

					// Se a instrução for ADDD
					if (_strcmpi(instruction.c_str(), InstructionType::ADDD.c_str()) == 0 )
					{
						this->instructions[i].instructionType = InstructionType::ADDD; // Define o tipo da instrução como ADDD
						read >> this->instructions[i].Rd; // Lê o registrador de destino
						read >> this->instructions[i].Rs; // Lê o primeiro registrador de origem
						read >> this->instructions[i].Rt; // Lê o segundo registrador de origem
					}

					// Se a instrução for SUBD
					else if (_strcmpi(instruction.c_str(), InstructionType::SUBD.c_str()) == 0)
					{
						this->instructions[i].instructionType = InstructionType::SUBD; // Define o tipo da instrução como SUBD
						read >> this->instructions[i].Rd; // Lê o registrador de destino
						read >> this->instructions[i].Rs; // Lê o primeiro registrador de origem
						read >> this->instructions[i].Rt; // Lê o segundo registrador de origem
					}

					// Se a instrução for MULTD
					else if (_strcmpi(instruction.c_str(), InstructionType::MULTD.c_str()) == 0)
					{
						this->instructions[i].instructionType = InstructionType::MULTD; // Define o tipo da instrução como MULTD
						read >> this->instructions[i].Rd; // Lê o registrador de destino
						read >> this->instructions[i].Rs; // Lê o primeiro registrador de origem
						read >> this->instructions[i].Rt; // Lê o segundo registrador de origem
					}

					// Se a instrução for DIVD
					else if (_strcmpi(instruction.c_str(), InstructionType::DIVD.c_str()) == 0)
					{
						this->instructions[i].instructionType = InstructionType::DIVD; // Define o tipo da instrução como DIVD
						read >> this->instructions[i].Rd; // Lê o registrador de destino
						read >> this->instructions[i].Rs; // Lê o primeiro registrador de origem
						read >> this->instructions[i].Rt; // Lê o segundo registrador de origem
					}

					// Se a instrução for LOAD
					else if (_strcmpi(instruction.c_str(), InstructionType::LOAD.c_str())==0)
					{
						this->instructions[i].instructionType = InstructionType::LOAD; // Define o tipo da instrução como LOAD
						read >> this->instructions[i].Rt; // Lê o registrador de destino
						read >> this->instructions[i].immediateOffset; // Lê o deslocamento imediato
						read >> this->instructions[i].Rs; // Lê o registrador de origem
					}

					// Se a instrução for STORE
					else if (_strcmpi(instruction.c_str(), InstructionType::STORE.c_str()) == 0)
					{
						this->instructions[i].instructionType = InstructionType::STORE; // Define o tipo da instrução como STORE
						read >> this->instructions[i].Rt; // Lê o registrador de destino
						read >> this->instructions[i].immediateOffset; // Lê o deslocamento imediato
						read >> this->instructions[i].Rs; // Lê o registrador de origem
					}
				}



	}
	// A função findFreeReservationStationForStoreInstruction procura por uma estação de reserva livre para uma instrução de armazenamento
	int findFreeReservationStationForStoreInstruction()
	{
		// Loop através de todos os buffers de armazenamento
		for (int i = 0; i < totalStoreBuffers; i++)
		{
			// Se o buffer de armazenamento atual não estiver ocupado, retorna o índice do buffer
			if (storeBuffers[i].isBusy == false)
				return i;
		}

		// Se todas as estações de reserva estiverem ocupadas, retorna -1
		return -1;
	}

	// A função findFreeReservationStationforAddSubInstruction procura por uma estação de reserva livre para uma instrução de adição/subtração
	int findFreeReservationStationforAddSubInstruction()
	{
		// Loop através de todas as estações de reserva de adição/subtração
		for (int i = 0; i < totalAddSubReservationStations; i++)
		{
			// Se a estação de reserva atual não estiver ocupada, retorna o índice da estação
			if (addSubReservationStations[i].isBusy == false)
			{
				return i;
			}
		}
		// Se todas as estações de reserva estiverem ocupadas, retorna -1
		return -1;
	}

	// A função findFreeReservationStationforMultDivInstruction procura por uma estação de reserva livre para uma instrução de multiplicação/divisão
	int findFreeReservationStationforMultDivInstruction()
	{
		// Loop através de todas as estações de reserva de multiplicação/divisão
		for (int i = 0; i < totalMultDivReservationStations; i++)
		{
			// Se a estação de reserva atual não estiver ocupada, retorna o índice da estação
			if (multDivReservationStations[i].isBusy == false)
			{
				return i;
			}
		}
		// Se todas as estações de reserva estiverem ocupadas, retorna -1
		return -1;
	}

	// A função findFreeRservationStationForLoadInstruction procura por uma estação de reserva livre para uma instrução de carga
	int findFreeRservationStationForLoadInstruction()
	{
		// Loop através de todos os buffers de carga
		for (int i = 0; i <totalLoadBuffers; i++)
		{
			// Se o buffer de carga atual não estiver ocupado, retorna o índice do buffer
			if (loadBuffers[i].isBusy == false)
			return i;

		}

		// Se todas as estações de reserva estiverem ocupadas, retorna -1
		return -1;

	}

	// A função broadCastResult transmite o resultado para todas as outras unidades que estão esperando este valor
	void broadCastResult(string val, string name)
	{
		// Loop através de todos os buffers de carga
		for (int i = 0; i < totalLoadBuffers; i++)
		{
			// Se o buffer de carga atual não estiver ocupado, pula para a próxima iteração do loop
			if (this->loadBuffers[i].isBusy == false)
				continue;

			// Se a unidade funcional do buffer de carga atual for a unidade que produziu o valor, limpa a unidade funcional
			if (this->loadBuffers[i].fu == name)
			{
				this->loadBuffers[i].fu = "";
			}
		}

		// Loop através de todos os buffers de armazenamento
		for (int i = 0; i < totalStoreBuffers; i++)
		{
			// Se o buffer de armazenamento atual não estiver ocupado, pula para a próxima iteração do loop
			if (this->storeBuffers[i].isBusy == false)
				continue;

			// Se a unidade funcional do buffer de armazenamento atual for a unidade que produziu o valor, limpa a unidade funcional
			if (this->storeBuffers[i].fu == name)
			{
				this->storeBuffers[i].fu = "";
			}
		}

		// Loop através de todas as estações de reserva de adição/subtração
		for (int i = 0; i < totalAddSubReservationStations; i++)
		{
			// Se a unidade que produziu o valor for a unidade que a estação de reserva atual está esperando, atualiza o valor e limpa a unidade
			if (this->addSubReservationStations[i].Qj == name)
			{
				// Define o campo Qj da i-ésima estação de reserva de adição/subtração como vazio
				this->addSubReservationStations[i].Qj = "";

				// Define o valor de Vj da i-ésima estação de reserva de adição/subtração como 'val'
				this->addSubReservationStations[i].Vj = val;

			}

			// Se a unidade que produziu o valor for a unidade que a estação de reserva atual está esperando, atualiza o valor e limpa a unidade
			if (this->addSubReservationStations[i].Qk == name)
			{
				this->addSubReservationStations[i].Qk = "";
				this->addSubReservationStations[i].Vk = val;
			}
		}

		// Loop através de todas as estações de reserva de multiplicação/divisão
		for (int i = 0; i < totalMultDivReservationStations; i++)
		{
			// Se a unidade que produziu o valor for a unidade que a estação de reserva atual está esperando, atualiza o valor e limpa a unidade
			if (this->multDivReservationStations[i].Qj == name)
			{
				this->multDivReservationStations[i].Qj = "";
				this->multDivReservationStations[i].Vj = val;

			}

			// Se a unidade que produziu o valor for a unidade que a estação de reserva atual está esperando, atualiza o valor e limpa a unidade
			if (this->multDivReservationStations[i].Qk == name)
			{
				this->multDivReservationStations[i].Qk = "";
				this->multDivReservationStations[i].Vk = val;
			}
		}
	}
	// Declaração da função issueInstruction que recebe um número de instrução como parâmetro
	int issueInstruction(int currentInstructionNumberToBeIssued)
	{
		int bufferNo;
		char num[10];
		_itoa_s(currentInstructionNumberToBeIssued, num, 10);

		// Se o número da instrução atual for menor que o total de instruções
		if (currentInstructionNumberToBeIssued < totalInstructions)
		{
			// Se o tipo da instrução atual for LOAD
			if (instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::LOAD)
			{
				// Encontra um buffer de reserva livre para a instrução de carga
				bufferNo = findFreeRservationStationForLoadInstruction();

				// Se não houver buffer de reserva livre
				if (bufferNo == -1)
				{
					// Adiciona uma mensagem ao log de razões indicando que a instrução não foi emitida devido a um perigo estrutural
					reason += "-> O número da instrução: ";
					reason.append(num);
					reason+=" não foi emitido devido ao perigo estrutural.\n";

					return -1;
				}
				else
				{
					// Se houver um buffer de reserva livre, define o buffer de carga como ocupado e associa a instrução ao buffer de carga
					// Define o buffer de carga como ocupado
					loadBuffers[bufferNo].isBusy = true;

					// Associa a instrução atual ao buffer de carga
					loadBuffers[bufferNo].instruction = &this->instructions[currentInstructionNumberToBeIssued];

					// Define o endereço do buffer de carga como o valor do registro Rs da instrução atual
					loadBuffers[bufferNo].address = this->instructions[currentInstructionNumberToBeIssued].Rs;

					// Converte o deslocamento imediato da instrução atual para uma string
					char offset[10];
					_itoa_s(this->instructions[currentInstructionNumberToBeIssued].immediateOffset, offset, 10);

					// Adiciona o deslocamento ao endereço do buffer de carga
					loadBuffers[bufferNo].address += " + ";
					loadBuffers[bufferNo].address.append(offset);

					// Define o ciclo de emissão da instrução como o ciclo atual
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.issue = currentCycleNumber;

					// Adiciona uma mensagem ao log indicando que a instrução foi emitida no buffer de carga
					reason += "-> O número da instrução: ";
					reason.append(num);
					reason += " foi emitido no Buffer de Carga= "+loadBuffers[bufferNo].name+"\n";

					// Define o número de ciclos restantes para a execução da instrução como o total de ciclos de carga/armazenamento
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.executionCyclesRemaining = this->totalLoadStoreCycles;

					// Verifica o perigo de leitura após gravação (RAW)
					int regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rt.c_str()[1]); // extrai o número do registro a partir do nome do registro
					this->loadBuffers[bufferNo].fu = this->registerStatus[regNumber].writingUnit;

					// Define o status do registro que será escrito por esta instrução de carga
					this->registerStatus[regNumber].writingUnit = loadBuffers[bufferNo].name;
				}
			}
			// Se o tipo da instrução atual for STORE
			else if (instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::STORE)
			{
				// Encontra um buffer de reserva livre para a instrução de armazenamento
				bufferNo = findFreeReservationStationForStoreInstruction();

				// Se não houver buffer de reserva livre
				if (bufferNo == -1)
				{
					// Adiciona uma mensagem ao log de razões indicando que a instrução não foi emitida devido a um perigo estrutural
					reason += "-> O numero da instrucao: ";
					reason.append(num);
					reason += " nao foi emitido devido ao perigo estrutural.\n";
					return -1;
				}
				else
				{
					// Se houver um buffer de reserva livre, define o buffer de armazenamento como ocupado e associa a instrução ao buffer de armazenamento
					reason += "-> O numero da instrucao: ";
					reason.append(num);
					reason += " foi emitido no Buffer de Armazenamento= " + storeBuffers[bufferNo].name + "\n";

					this->storeBuffers[bufferNo].isBusy = true; // Define o buffer de armazenamento como ocupado

					// Verifica o perigo RAW (Read After Write)
					int regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rt.c_str()[1]);
					this->storeBuffers[bufferNo].fu = this->registerStatus[regNumber].writingUnit; // Associa a unidade de escrita do registro ao buffer de armazenamento

					// Define o número de ciclos restantes para a execução da instrução
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.executionCyclesRemaining=this->totalLoadStoreCycles;

					storeBuffers[bufferNo].isBusy = true; // Define o buffer de armazenamento como ocupado
					storeBuffers[bufferNo].instruction = &this->instructions[currentInstructionNumberToBeIssued]; // Associa a instrução ao buffer de armazenamento
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.issue = currentCycleNumber; // Define o ciclo atual como o ciclo de emissão da instrução
					storeBuffers[bufferNo].address = this->instructions[currentInstructionNumberToBeIssued].Rs; // Define o endereço do buffer de armazenamento como o registrador de origem da instrução
					char offset[10];
					_itoa_s(this->instructions[currentInstructionNumberToBeIssued].immediateOffset, offset, 10); // Converte o deslocamento imediato da instrução para string
					storeBuffers[bufferNo].address += " + ";
					storeBuffers[bufferNo].address.append(offset); // Adiciona o deslocamento imediato ao endereço do buffer de armazenamento
				}
			}
			// Se o tipo da instrução atual for ADDD ou SUBD
			else if (instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::ADDD || instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::SUBD)
			{
				// Encontra um buffer de reserva livre para a instrução de adição ou subtração
				int bufferNo = findFreeReservationStationforAddSubInstruction();

				// Se não houver buffer de reserva livre
				if (bufferNo == -1)
				{
					// Adiciona uma mensagem ao log de razões indicando que a instrução não pode ser emitida devido a um perigo estrutural
					reason += "-> A instrução No: ";
					reason.append(num);
					reason += " nao pode ser emitida devido ao perigo estrutural!\n";
					return -1;
				}
				else
				{
					// Se houver um buffer de reserva livre, define o buffer de reserva como ocupado e associa a instrução ao buffer de reserva
					reason += "-> A instrução No: ";
					reason.append(num);
					reason += " foi emitida na Estação de Reserva= "+addSubReservationStations[bufferNo].name+ "\n";

					// Define o número de ciclos restantes para a execução da instrução como o total de ciclos de adição/subtração
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.executionCyclesRemaining = this->totalAddSubCycles;

					// Define o tipo de instrução na estação de reserva de adição/subtração
					this->addSubReservationStations[bufferNo].instructionType = this->instructions[currentInstructionNumberToBeIssued].instructionType;

					// Associa a instrução à estação de reserva de adição/subtração
					this->addSubReservationStations[bufferNo].instruction = &this->instructions[currentInstructionNumberToBeIssued];

					// Define a estação de reserva de adição/subtração como ocupada
					this->addSubReservationStations[bufferNo].isBusy = true;

					// Define o ciclo de emissão da instrução como o ciclo atual
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.issue = currentCycleNumber;
					// Verifica o perigo RAW
					// RS
					// Converte o número do registro Rs da instrução atual para um inteiro
					int regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rs.c_str()[1]);

					// Define o campo Qj da estação de reserva de adição/subtração como a unidade de escrita do registro
					this->addSubReservationStations[bufferNo].Qj = this->registerStatus[regNumber].writingUnit;

					// Rt
					regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rt.c_str()[1]);
					this->addSubReservationStations[bufferNo].Qk = this->registerStatus[regNumber].writingUnit;

					// Se o campo Qj da estação de reserva de adição/subtração estiver vazio,
					// define o valor de Vj como o valor do registro Rs da instrução atual
					if (this->addSubReservationStations[bufferNo].Qj == "")
					{
						this->addSubReservationStations[bufferNo].Vj = "R(" + this->instructions[currentInstructionNumberToBeIssued].Rs + ")";
					}

					// Se o campo Qk da estação de reserva de adição/subtração estiver vazio,
					// define o valor de Vk como o valor do registro Rt da instrução atual
					if (this->addSubReservationStations[bufferNo].Qk == "")
					{
						this->addSubReservationStations[bufferNo].Vk = "R(" + this->instructions[currentInstructionNumberToBeIssued].Rt + ")";
					}

					// Define o status do registro
					regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rd.c_str()[1]);
					this->registerStatus[regNumber].writingUnit = this->addSubReservationStations[bufferNo].name;
				}
			}
			// Se o tipo da instrução atual for MULTD ou DIVD
			else if (instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::MULTD || instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::DIVD)
			{
				// Encontra um buffer de reserva livre para a instrução de multiplicação ou divisão
				int bufferNo = findFreeReservationStationforMultDivInstruction();
				char instructionNumberConversion[10];
				_itoa_s(currentInstructionNumberToBeIssued, instructionNumberConversion, 10, 10);

				// Se não houver buffer de reserva livre
				if (bufferNo == -1)
				{
					// Adiciona uma mensagem ao log de razões indicando que a instrução não pode ser emitida devido a um perigo estrutural
					reason += "-> A instrução No: ";
					reason.append(instructionNumberConversion);
					reason += " nao pode ser emitida devido ao perigo estrutural!\n";
					return -1;
				}
				else
				{
					// Se houver um buffer de reserva livre, define o buffer de reserva como ocupado e associa a instrução ao buffer de reserva
					reason += "-> A instrução No: ";
					reason.append(num);
					reason += " foi emitida na Estação de Reserva= " + this->multDivReservationStations[bufferNo].name + "\n";

					// Se o tipo de instrução for MULTD, define o número de ciclos restantes para a execução da instrução como o total de ciclos de multiplicação
					// Caso contrário, define como o total de ciclos de divisão
					if (this->instructions[currentInstructionNumberToBeIssued].instructionType == InstructionType::MULTD)
						this->instructions[currentInstructionNumberToBeIssued].instructionStatus.executionCyclesRemaining = this->totalMultCylcles;
					else
						this->instructions[currentInstructionNumberToBeIssued].instructionStatus.executionCyclesRemaining = this->totalDivCycles;

					// Define o tipo de instrução na estação de reserva de multiplicação/divisão
					this->multDivReservationStations[bufferNo].instructionType = this->instructions[currentInstructionNumberToBeIssued].instructionType;

					// Associa a instrução à estação de reserva de multiplicação/divisão
					this->multDivReservationStations[bufferNo].instruction = &this->instructions[currentInstructionNumberToBeIssued];

					// Define a estação de reserva de multiplicação/divisão como ocupada
					this->multDivReservationStations[bufferNo].isBusy = true;

					// Define o ciclo de emissão da instrução como o ciclo atual
					this->instructions[currentInstructionNumberToBeIssued].instructionStatus.issue = currentCycleNumber;

					// Verifica o perigo de leitura após gravação (RAW)
					// Para o registro Rs
					int regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rs.c_str()[1]);
					this->multDivReservationStations[bufferNo].Qj = this->registerStatus[regNumber].writingUnit;

					// Para o registro Rt
					regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rt.c_str()[1]);
					this->multDivReservationStations[bufferNo].Qk = this->registerStatus[regNumber].writingUnit;

					// Se o campo Qj da estação de reserva de multiplicação/divisão estiver vazio,
					// define o valor de Vj como o valor do registro Rs da instrução atual
					if (this->multDivReservationStations[bufferNo].Qj == "")
					{
						this->multDivReservationStations[bufferNo].Vj = "R(" + this->instructions[currentInstructionNumberToBeIssued].Rs + ")";
					}

					// Se o campo Qk da estação de reserva de multiplicação/divisão estiver vazio,
					// define o valor de Vk como o valor do registro Rt da instrução atual
					if (this->multDivReservationStations[bufferNo].Qk == "")
					{
						this->multDivReservationStations[bufferNo].Vk = "R(" + this->instructions[currentInstructionNumberToBeIssued].Rt + ")";
					}

					// Define o status do registro que será escrito por esta instrução de multiplicação/divisão
					regNumber = atoi(&this->instructions[currentInstructionNumberToBeIssued].Rd.c_str()[1]);
					this->registerStatus[regNumber].writingUnit = this->multDivReservationStations[bufferNo].name;
				}
			}
		}
		return 0;
	}
// A função writeBack é responsável pela etapa de escrita de volta do ciclo de execução de instrução
void writeBack()
{
	// Loop através de todos os buffers de carga
	for (int i = 0; i < totalLoadBuffers; i++)
	{
		// Variável estática para manter o controle do número de instruções
		int static num = 1;

		// Se o buffer de carga atual não estiver ocupado, pula para a próxima iteração do loop
		if (this->loadBuffers[i].isBusy == false)
			continue;

		// Se a instrução no buffer de carga atual ainda tiver ciclos de execução restantes, pula para a próxima iteração do loop
		if (this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining != 0)
			continue; // a instrução ainda está sendo executada

		// Se a instrução no buffer de carga atual tiver completado a execução no ciclo atual, pula para a próxima iteração do loop
		if (this->loadBuffers[i].instruction->instructionStatus.executionComplete == currentCycleNumber)
			continue; // a instrução completou a execução no ciclo atual, então não pode ser escrita no ciclo atual

		// Define o ciclo atual como o ciclo de escrita da instrução no buffer de carga atual
		this->loadBuffers[i].instruction->instructionStatus.writeBack = currentCycleNumber;

		// Adiciona uma mensagem ao log de razões indicando que a instrução no buffer de carga atual foi escrita
		reason += "-> A instrução no Buffer de Carga= " + this->loadBuffers[i].name + " foi escrita.\n";

		// Obtém o número do registro da instrução no buffer de carga atual
		int regNum=atoi(&this->loadBuffers[i].instruction->Rt.c_str()[1]);

		// Se a unidade de escrita do registro for o buffer de carga atual, limpa a unidade de escrita do registro
		if (this->registerStatus[regNum].writingUnit == this->loadBuffers[i].name)
			this->registerStatus[regNum].writingUnit = "";

		// Define o buffer de carga atual como não ocupado
		this->loadBuffers[i].isBusy = false;

		// Limpa o endereço do buffer de carga atual
		this->loadBuffers[i].address = "";

		// Define a instrução do buffer de carga atual como nula
		this->loadBuffers[i].instruction = nullptr;

		// Converte o número da instrução para uma string e a anexa a "M(A" para criar o nome do valor
		char numChar[10];
		_itoa_s(num, numChar, 10);
		string val = "M(A";
		val.append(numChar);
		val += ")";

		// Transmite o resultado para todas as outras unidades que estão esperando este valor
		broadCastResult(val, this->loadBuffers[i].name);

		// Incrementa o número da instrução
		num++;
	}

		// Loop através de todos os buffers de armazenamento
		for (int i = 0; i < totalStoreBuffers; i++)
		{
			// Se o buffer de armazenamento atual não estiver ocupado, pula para a próxima iteração do loop
			if (this->storeBuffers[i].isBusy == false)
				continue;

			// Se a instrução no buffer de armazenamento atual ainda tiver ciclos de execução restantes, pula para a próxima iteração do loop
			if (this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining != 0)
				continue; // a instrução ainda está sendo executada

			// Se a instrução no buffer de armazenamento atual tiver completado a execução no ciclo atual, pula para a próxima iteração do loop
			if (this->storeBuffers[i].instruction->instructionStatus.executionComplete == currentCycleNumber)
				continue; // a instrução completou a execução no ciclo atual, então não pode ser escrita no ciclo atual

			// Adiciona uma mensagem ao log de razões indicando que a instrução no buffer de armazenamento atual foi escrita
			reason += "-> A instrução no Buffer de Armazenamento= " + this->storeBuffers[i].name + " foi escrita.\n";
			// Define o ciclo atual como o ciclo de escrita da instrução no buffer de armazenamento atual
			this->storeBuffers[i].instruction->instructionStatus.writeBack = currentCycleNumber;
			// Define o buffer de armazenamento atual como não ocupado
			this->storeBuffers[i].isBusy = false;
			// Limpa o endereço do buffer de armazenamento atual
			this->storeBuffers[i].address = "";
			// Define a instrução do buffer de armazenamento atual como nula
			this->storeBuffers[i].instruction = nullptr;
			// Transmite o resultado para todas as outras unidades que estão esperando este valor
			//broadCastResult();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////


		// Loop através de todas as estações de reserva de adição/subtração
		for (int i = 0; i < totalAddSubReservationStations; i++)
		{
			// Variável estática para manter o controle do número de instruções
			int static num = 1;

			// Se a estação de reserva atual não estiver ocupada, pula para a próxima iteração do loop
			if (this->addSubReservationStations[i].isBusy == false)
				continue;

			// Se a instrução na estação de reserva atual ainda tiver ciclos de execução restantes, pula para a próxima iteração do loop
			if (this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining != 0)
				continue; // a instrução ainda está sendo executada

			// Se a instrução na estação de reserva atual tiver completado a execução no ciclo atual, pula para a próxima iteração do loop
			if (this->addSubReservationStations[i].instruction->instructionStatus.executionComplete == currentCycleNumber)
				continue; // a instrução completou a execução no ciclo atual, então não pode ser escrita no ciclo atual

			// Define o ciclo atual como o ciclo de escrita da instrução na estação de reserva atual
			this->addSubReservationStations[i].instruction->instructionStatus.writeBack = currentCycleNumber;
			// Adiciona uma mensagem ao log de razões indicando que a instrução na estação de reserva atual foi escrita
			reason += "-> A instrução na Estação de Reserva= " + this->addSubReservationStations[i].name + " foi escrita.\n";

			// Obtém o número do registro da instrução na estação de reserva atual
			int regNum = atoi(&this->addSubReservationStations[i].instruction->Rd.c_str()[1]);
			// Se a unidade de escrita do registro for a estação de reserva atual, limpa a unidade de escrita do registro
			if (this->registerStatus[regNum].writingUnit == this->addSubReservationStations[i].name)
				this->registerStatus[regNum].writingUnit = "";

			// Define a estação de reserva atual como não ocupada
			this->addSubReservationStations[i].isBusy = false;
			// Limpa o tipo de instrução da estação de reserva atual
			this->addSubReservationStations[i].instructionType = "";
			// Limpa os campos Qj, Qk, Vj, Vk da estação de reserva atual
			this->addSubReservationStations[i].Qj = "";
			this->addSubReservationStations[i].Qk = "";
			this->addSubReservationStations[i].Vj = "";
			this->addSubReservationStations[i].Vk = "";
			// Define a instrução da estação de reserva atual como nula
			this->addSubReservationStations[i].instruction = nullptr;

			// Converte o número da instrução para uma string e a anexa a "V" para criar o nome do valor
			char numChar[10];
			_itoa_s(num, numChar, 10);
			string val = "V";
			val.append(numChar);

			// Transmite o resultado para todas as outras unidades que estão esperando este valor
			broadCastResult(val, this->addSubReservationStations[i].name);
			// Incrementa o número da instrução
			num++;
		}
		////////////////////////////////////////////////////////////////////////////////

		// Loop para percorrer todas as estações de reserva para multiplicação/divisão
		for (int i = 0; i < totalMultDivReservationStations; i++)
		{
		    int static num = 1;
			// Se a estação de reserva atual não estiver ocupada, pula para a próxima iteração do loop
			if (this->multDivReservationStations[i].isBusy == false)
				continue;

			// Se a instrução na estação de reserva atual ainda tiver ciclos de execução restantes, pula para a próxima iteração do loop
			if (this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining != 0)
				continue;

			// Se a instrução na estação de reserva atual tiver completado a execução no ciclo atual, pula para a próxima iteração do loop
			if (this->multDivReservationStations[i].instruction->instructionStatus.executionComplete == currentCycleNumber)
				continue;

			// Define o ciclo atual como o ciclo de escrita da instrução na estação de reserva atual
			this->multDivReservationStations[i].instruction->instructionStatus.writeBack = currentCycleNumber;

			// Adiciona uma mensagem ao log de razões indicando que a instrução na estação de reserva atual foi escrita
			reason += "-> A instrução na Estação de Reserva= " + this->multDivReservationStations[i].name + " foi escrita.\n";

			// Obtém o número do registro da instrução na estação de reserva atual
			int regNum = atoi(&this->multDivReservationStations[i].instruction->Rd.c_str()[1]);

			// Se a unidade de escrita do registro for a estação de reserva atual, limpa a unidade de escrita do registro
			if (this->registerStatus[regNum].writingUnit == this->multDivReservationStations[i].name)
				this->registerStatus[regNum].writingUnit = "";

			// Define a estação de reserva atual como não ocupada
			this->multDivReservationStations[i].isBusy = false;
			this->multDivReservationStations[i].instructionType = "";
			this->multDivReservationStations[i].Qj = "";
			this->multDivReservationStations[i].Qk = "";
			this->multDivReservationStations[i].Vj = "";
			this->multDivReservationStations[i].Vk = "";
			this->multDivReservationStations[i].instruction = nullptr;

			char numChar[10];
			_itoa_s(num, numChar, 10);
			string val = "V";
			val.append(numChar);

			broadCastResult(val, this->multDivReservationStations[i].name);
			num++;
		}

}



	// A função execute é responsável por executar as instruções nos buffers de carga
	void execute()
	{
		// O loop for percorre todos os buffers de carga
		for (int i = 0; i < totalLoadBuffers; i++)
		{
			// Se o buffer de carga atual não estiver ocupado, ele pula para a próxima iteração do loop
			if (this->loadBuffers[i].isBusy == false)
				continue;

			// Se a unidade funcional do buffer de carga atual não estiver vazia, ele pula para a próxima iteração do loop
			// Isso ocorre porque a execução ainda não começou devido a um perigo RAW (Read After Write)
			if (this->loadBuffers[i].fu != "")
				continue;

			// Se a execução da instrução no buffer de carga atual ainda não começou
			if (this->loadBuffers[i].instruction->instructionStatus.executionStart == -1)
			{
				// Se a instrução foi emitida no ciclo atual, ela não pode começar a execução no mesmo ciclo
				if (this->loadBuffers[i].instruction->instructionStatus.issue == currentCycleNumber)
					continue;

				// A execução da instrução começa
				this->loadBuffers[i].instruction->instructionStatus.executionStart = currentCycleNumber;
				this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining--;

				// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
				if (this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining == 0)
				{
					this->loadBuffers[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
					reason += "-> A instrução no Buffer de Carga= " + this->loadBuffers[i].name + " concluiu a execução.\n";
					continue;
				}
				else
				{
					reason += "-> A instrução no Buffer de Carga= " + this->loadBuffers[i].name + " começou a execução.\n";
					continue;
				}
			}

			// Se ainda há ciclos de execução restantes, eles são decrementados
			if (this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining!=0)
				this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining--;
			else continue;

			// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
			if (this->loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining == 0)
			{
				this->loadBuffers[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
				reason += "-> A instrução no Buffer de Carga= " + this->loadBuffers[i].name + " concluiu a execução.\n";
				continue;
			}
			else
			{
				reason += "-> A instrução no Buffer de Carga= " + this->loadBuffers[i].name + " completou mais um ciclo de execução.\n";
				continue;
			}

		} // Fim do loop dos buffers de carga


// O loop for percorre todos os buffers de armazenamento
for (int i = 0; i < totalStoreBuffers; i++)
{
	// Se o buffer de armazenamento atual não estiver ocupado, ele pula para a próxima iteração do loop
	if (this->storeBuffers[i].isBusy == false)
		continue;

	// Se a unidade funcional do buffer de armazenamento atual não estiver vazia, ele pula para a próxima iteração do loop
	// Isso ocorre porque a execução ainda não começou devido a um perigo RAW (Read After Write)
	if (this->storeBuffers[i].fu != "")
		continue;

	// Se a execução da instrução no buffer de armazenamento atual ainda não começou
	if (this->storeBuffers[i].instruction->instructionStatus.executionStart == -1)
	{
		// Se a instrução foi emitida no ciclo atual, ela não pode começar a execução no mesmo ciclo
		if (this->storeBuffers[i].instruction->instructionStatus.issue == currentCycleNumber)
			continue;

		// A execução da instrução começa
		this->storeBuffers[i].instruction->instructionStatus.executionStart = currentCycleNumber;
		this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining--;

		// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
		if (this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining == 0)
		{
			this->storeBuffers[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
			reason += "-> A instrução no Store Buffer= " + this->storeBuffers[i].name + " concluiu a execução.\n";
			continue;
		}
		else
		{
			reason += "-> A instrução no Store Buffer= " + this->storeBuffers[i].name + " começou a execução.\n";
			continue;
		}
	}

	// Se ainda há ciclos de execução restantes, eles são decrementados
	if (this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining!=0)
		this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining--;
	else continue;

	// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
	if (this->storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining == 0)
	{
		this->storeBuffers[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
		reason += "-> A instrução no Store Buffer= " + this->storeBuffers[i].name + " concluiu a execução.\n";
		continue;
	}
	else
	{
		reason += "-> A instrução no Store Buffer= " + this->storeBuffers[i].name + " completou mais um ciclo de execução.\n";
		continue;
	}

} // Fim do loop dos buffers de armazenamento

		// Este loop percorre todas as estações de reserva de adição/subtração
for (int i = 0; i < totalAddSubReservationStations; i++)
{
    // Se a estação de reserva atual não estiver ocupada, pula para a próxima iteração do loop
    if (this->addSubReservationStations[i].isBusy == false)
        continue;

    // Se a estação de reserva atual estiver esperando por uma ou mais unidades funcionais, pula para a próxima iteração do loop
    // Isso é feito para evitar um perigo de dados RAW (Read After Write)
    if (this->addSubReservationStations[i].Qj!="" || this->addSubReservationStations[i].Qk!="")
        continue;

    // Se a execução da instrução na estação de reserva atual ainda não começou
    if (this->addSubReservationStations[i].instruction->instructionStatus.executionStart == -1)
    {
        // Se a instrução foi emitida no ciclo atual, ela não pode começar a execução no mesmo ciclo
        if (this->addSubReservationStations[i].instruction->instructionStatus.issue == currentCycleNumber)
            continue;

        // A execução da instrução começa
        this->addSubReservationStations[i].instruction->instructionStatus.executionStart = currentCycleNumber;
        this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining--;

        // Se a execução da instrução foi concluída
        if (this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining == 0)
        {
            // Marca o ciclo atual como o ciclo em que a execução foi concluída
            this->addSubReservationStations[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
            reason += "-> A instrução na Estação de Reserva= " + this->addSubReservationStations[i].name + " completou a execução.\n";
            continue;
        }
        else
        {
            // A execução da instrução começou, mas ainda não foi concluída
            reason += "-> A instrução na Estação de Reserva= " + this->addSubReservationStations[i].name + " começou a execução.\n";
            continue;
        }
    }

    // Se a execução da instrução ainda não foi concluída, decrementa o número de ciclos restantes
    if (this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining!=0)
        this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining--;
    else continue;

    // Se a execução da instrução foi concluída
    if (this->addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining == 0)
    {
        // Marca o ciclo atual como o ciclo em que a execução foi concluída
        this->addSubReservationStations[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
        reason += "-> A instrução na Estação de Reserva= " + this->addSubReservationStations[i].name + " completou a execução.\n";
        continue;
    }
    else
    {
        // A execução da instrução ainda não foi concluída, mas completou mais um ciclo
        reason += "-> A instrução na Estação de Reserva= " + this->addSubReservationStations[i].name + " completou mais um ciclo de execução.\n";
        continue;
    }
}//fim do loop das Estaçoes de Reserva addSubss



		////////////////////////////////////////////////////////////////////////
		// O loop for percorre todas as estações de reserva multiDiv
		for (int i = 0; i < totalMultDivReservationStations; i++)
		{
			// Se a estação de reserva multiDiv atual não estiver ocupada, ele pula para a próxima iteração do loop
			if (this->multDivReservationStations[i].isBusy == false)
				continue;

			// Se Qj ou Qk da estação de reserva multiDiv atual não estiverem vazios, ele pula para a próxima iteração do loop
			// Isso ocorre porque a execução ainda não começou devido a um perigo RAW (Read After Write)
			if (this->multDivReservationStations[i].Qj != "" || this->multDivReservationStations[i].Qk != "")
				continue;

			// Se a execução da instrução na estação de reserva multiDiv atual ainda não começou
			if (this->multDivReservationStations[i].instruction->instructionStatus.executionStart == -1)
			{
				// Se a instrução foi emitida no ciclo atual, ela não pode começar a execução no mesmo ciclo
				if (this->multDivReservationStations[i].instruction->instructionStatus.issue == currentCycleNumber)
					continue;

				// A execução da instrução começa
				this->multDivReservationStations[i].instruction->instructionStatus.executionStart = currentCycleNumber;
				this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining--;

				// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
				if (this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining == 0)
				{
					this->multDivReservationStations[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
					reason += "-> A instrução na Estação de Reserva= " + this->multDivReservationStations[i].name + " Completou a execução.\n";
					continue;
				}
				else
				{
					reason += "-> A instrução na Estação de Reserva= " + this->multDivReservationStations[i].name + " completou mais um ciclo de execução.\n";
					continue;
				}
			}

			// Se ainda há ciclos de execução restantes, eles são decrementados
			if (this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining!=0)
				this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining--;
			else continue;

			// Se não há mais ciclos de execução restantes, a execução da instrução é concluída
			if (this->multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining == 0)
			{
				this->multDivReservationStations[i].instruction->instructionStatus.executionComplete = currentCycleNumber;
				reason += "-> A instrução na Estação de Reserva = " + this->multDivReservationStations[i].name + " Completou a execução.\n";
				continue;
			}
			else
			{
				reason += "-> A instrução na Estação de Reserva= " + this->multDivReservationStations[i].name + " completou mais um ciclo de execução.\n";
				continue;
			}

		} // Fim do loop das estações de reserva multiDiv
	}

	void Simulate()
	{
		// Inicializa o número da instrução atual a ser emitida e o número do ciclo atual
		int currentInstructionNumberToBeIssued = 0;
		currentCycleNumber = 0;

		// O loop while é executado indefinidamente
		while (true)
		{
			// Move o cursor para a posição (0,0) na tela
			gotoxy(0, 0);
			// Imprime o número do ciclo atual
			cout << "Numero Ciclo Atual= " << currentCycleNumber;
			// Chama a função print para imprimir o estado atual do simulador
			print();
			// Lê um caractere do usuário
			char a;
			cin >> a;
			// Incrementa o número do ciclo atual
			currentCycleNumber++;
			// Limpa a tela
			system("cls");
			// Limpa a string de razão
			reason = "";

			// Emite a instrução atual
			int flag = issueInstruction(currentInstructionNumberToBeIssued);

			// Se a instrução foi emitida com sucesso, incrementa o número da instrução a ser emitida
			if (flag != -1)
				currentInstructionNumberToBeIssued++;

			// Executa as instruções nas estações de reserva
			execute();
			// Escreve os resultados das instruções executadas de volta aos registradores ou memória
			writeBack();
		}
	}

	void print()
	{
		int y = 2;

		gotoxy(2, y);
		cout << "Instrucoes:";
		gotoxy(27, y);
		cout << "  Emitir" << "   Inicio" << "Fim     Escrevendo";
		gotoxy(27, y + 1);

		cout << "__________________________________";

		int j = 0;
		// Loop para percorrer todas as instruções
		for (int i = 0; i < totalInstructions; i++)
		{
			// Cria um array de caracteres
			char arr[10];
			// Converte o número da instrução para uma string
			_itoa_s(i, arr, 10);
			// Adiciona um ponto ao final da string
			strcat_s(arr, ".");

			// Move o cursor para uma posição específica na tela
			gotoxy(2, j + y + 2);
			// Verifica se o tipo de instrução é LOAD ou STORE
			if (instructions[i].instructionType == InstructionType::LOAD || instructions[i].instructionType == InstructionType::STORE)
				// Imprime a instrução formatada
				cout <<std::left<<setw(3)<<arr<< std::left << setw(5) << instructions[i].instructionType << setfill(' ') << std::right << setw(4) << instructions[i].Rt << setw(5) << instructions[i].immediateOffset << "+" << setw(5) << instructions[i].Rs;
			else
			{
				// Imprime a instrução formatada
				cout <<std::left << setw(3) <<arr<< std::left << setw(5) << instructions[i].instructionType << std::right << setw(4) << instructions[i].Rd << setw(5) << instructions[i].Rs << setw(6) << instructions[i].Rt;
			}

			// Move o cursor para uma posição específica na tela
			gotoxy(2 + 24, j + y + 2);
			j++;

			// Cria arrays de caracteres para armazenar os estados das instruções
			char instructionIssue[10];
			char executionStart[10];
			char executionComplete[10];
			char instructionWrite[10];

			// Converte os estados das instruções para strings
			_itoa_s(this->instructions[i].instructionStatus.issue, instructionIssue, 10);
			_itoa_s(this->instructions[i].instructionStatus.executionStart, executionStart, 10);
			_itoa_s(this->instructions[i].instructionStatus.executionComplete, executionComplete, 10);
			_itoa_s(this->instructions[i].instructionStatus.writeBack, instructionWrite, 10);

			// Imprime os estados das instruções formatados
			cout << "|" << std::right << setw(7) << (instructions[i].instructionStatus.issue==-1 ? "" : instructionIssue) << "|" << setw(7) << (instructions[i].instructionStatus.executionStart==-1 ? "" : executionStart) << "|" << setw(7) << (instructions[i].instructionStatus.executionComplete==-1 ? "" :executionComplete) << "|" << setw(10) << (instructions[i].instructionStatus.writeBack==-1 ? "" : instructionWrite) << "|";

			// Move o cursor para uma posição específica na tela
			gotoxy(2 + 24, j + y + 2);

			// Imprime uma linha de separação
			cout << "|_______|_______|_______|__________|";
			j++;
		}

		//printing Load Store Buffer
		gotoxy(70, y);
		cout << "Carreg/Armzr Buffer:Ocupado     Addr      FU";
		y++;
		gotoxy(66 + 22 + 2, y);
		cout << "_____________________________";
		// Loop para percorrer todos os buffers de carga
		for (int i = 0; i < totalLoadBuffers; i++)
		{
			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (76 + 2, y) na tela
			gotoxy(76 + 2, y);

			// Imprime na tela o nome do buffer de carga
			cout << setw(10) << std::right << loadBuffers[i].name;

			// Imprime um espaço
			cout << " ";

			// Imprime na tela se o buffer de carga está ocupado ou não, o endereço e a unidade funcional
			cout << "|" << std::right << setw(7) << (loadBuffers[i].isBusy == true ? "Sim" : "Nao") << "|" << setw(10) << loadBuffers[i].address << "|" << setw(10) << loadBuffers[i].fu << "|";

			// Se a instrução no buffer de carga atual não for nula
			if (loadBuffers[i].instruction != nullptr)
			{
				// Imprime na tela o número de ciclos de execução restantes da instrução
				cout << setw(3) << loadBuffers[i].instruction->instructionStatus.executionCyclesRemaining;
			}

			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (76 + 2 + 11, y) na tela
			gotoxy(76 + 2 + 11, y);

			// Imprime na tela uma linha de separação
			cout << "|_______|__________|__________|" << endl;
		}

		// Loop para percorrer todos os buffers de armazenamento
		for (int i = 0; i < totalStoreBuffers; i++)
		{
			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (76 + 2, y) na tela
			gotoxy(76 + 2, y);

			// Imprime na tela o nome do buffer de armazenamento
			cout << setw(10) << setfill(' ') << std::right << storeBuffers[i].name;

			// Imprime um espaço
			cout << " ";

			// Imprime na tela se o buffer de armazenamento está ocupado ou não, o endereço e a unidade funcional
			cout << "|" << std::right << setw(7) << (storeBuffers[i].isBusy == true ? "Sim" : "Nao") << "|" << setw(10) << storeBuffers[i].address << "|" << setw(10) << storeBuffers[i].fu << "|";

			// Se a instrução no buffer de armazenamento atual não for nula
			if (storeBuffers[i].instruction != nullptr)
			{
				// Imprime na tela o número de ciclos de execução restantes da instrução
				cout << setw(3) << storeBuffers[i].instruction->instructionStatus.executionCyclesRemaining;
			}

			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (76 + 2 + 11, y) na tela
			gotoxy(76 + 2 + 11, y);

			// Imprime na tela uma linha de separação
			cout << "|_______|__________|__________|" << endl;
		}


		//Estacoes reserva
		// Atribui a 'y' o maior valor entre 'y' e 'j'
		y = (y > j ? y : j);

		// Incrementa 'y' em 5
		y += 5;

		// Define a posição do cursor na tela para a coluna 4 e linha 'y'
		gotoxy(4, y);

		// Imprime "Estações Reserva:" na posição atual do cursor
		cout << "Estacoes Reserva:";

		// Incrementa 'y'
		y++;

		// Define a posição do cursor na tela para a coluna 21 e linha 'y'
		gotoxy(21, y);

		// Imprime o cabeçalho da tabela de estações de reserva
		cout << " Nome   Ocupado    Op      Vj       Vk       Qj       Qk ";

		// Incrementa 'y'
		y++;

		// Define a posição do cursor na tela para a coluna 28 e linha 'y'
		gotoxy(21 + 7, y);

		// Imprime uma linha de sublinhado para separar o cabeçalho do corpo da tabela
		cout << "_________________________________________________";
		// Loop para percorrer todas as estações de reserva de adição/subtração
		for (int i = 0; i < totalAddSubReservationStations; i++)
		{
			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (19, y) na tela
			gotoxy(19, y);

			// Imprime na tela o nome da estação de reserva, se está ocupada ou não, o tipo de instrução, Vj, Vk, Qj e Qk
			// A função setw() é usada para alinhar a saída
			cout << std::right << setw(7) << addSubReservationStations[i].name << " |" << setw(5) << (addSubReservationStations[i].isBusy == true ? "Sim" : "Nao") << "|" << setw(7) << addSubReservationStations[i].instructionType << "|" << setw(8) << addSubReservationStations[i].Vj << "|" << setw(8) << addSubReservationStations[i].Vk << "|" << setw(8) << addSubReservationStations[i].Qj << "|" << setw(8) << addSubReservationStations[i].Qk << "|";

			// Se a instrução na estação de reserva atual não for nula
			if (addSubReservationStations[i].instruction != nullptr)
			{
				// Imprime na tela o número de ciclos de execução restantes da instrução
				cout << setw(3) << addSubReservationStations[i].instruction->instructionStatus.executionCyclesRemaining;
			}

			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (19 + 7 + 1, y) na tela
			gotoxy(19 + 7 + 1, y);

			// Imprime na tela uma linha de separação
			cout << "|_____|_______|________|________|________|________|";
		}

		// Loop para percorrer todas as estações de reserva de multiplicação/divisão
		for (int i = 0; i < totalMultDivReservationStations; i++)
		{
			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (19, y) na tela
			gotoxy(19, y);

			// Imprime na tela o nome da estação de reserva, se está ocupada ou não, o tipo de instrução, Vj, Vk, Qj e Qk
			// A função setw() é usada para alinhar a saída
			cout << std::right << setw(7) << multDivReservationStations[i].name << " |" << setw(5) << (multDivReservationStations[i].isBusy == true ? "Sim" : "nao") << "|" << setw(7) << multDivReservationStations[i].instructionType << "|" << setw(8) << multDivReservationStations[i].Vj << "|" << setw(8) << multDivReservationStations[i].Vk << "|" << setw(8) << multDivReservationStations[i].Qj << "|" << setw(8) << multDivReservationStations[i].Qk << "|";

			// Se a instrução na estação de reserva atual não for nula
			if (multDivReservationStations[i].instruction != nullptr)
			{
				// Imprime na tela o número de ciclos de execução restantes da instrução
				cout << setw(3) << multDivReservationStations[i].instruction->instructionStatus.executionCyclesRemaining;
			}

			// Incrementa a variável y
			y++;

			// Move o cursor para a posição (19 + 7 + 1, y) na tela
			gotoxy(19 + 7 + 1, y);

			// Imprime na tela uma linha de separação
			cout << "|_____|_______|________|________|________|________|";
		}
		y+=3;
		int x = 19;


		// Loop para percorrer todos os registros
		for (int i = 0; i < totalRegisters; i++)
		{
			// Move o cursor para a posição (x, y) na tela
			gotoxy(x, y);

			// Imprime na tela o nome do registro
			cout << std::right << setw(5) << registerStatus[i].registerName;

			// Move o cursor para a posição (x+1, y+1) na tela
			gotoxy(x+1, y + 1);

			// Imprime na tela uma linha de separação
			cout << "________";

			// Move o cursor para a posição (x, y+2) na tela
			gotoxy(x, y + 2);

			// Imprime na tela a unidade que está escrevendo no registro
			cout << "|" << setw(5) << registerStatus[i].writingUnit;

			// Move o cursor para a posição (x, y+3) na tela
			gotoxy(x, y + 3);

			// Imprime na tela uma linha de separação
			cout << "|________";

			// Incrementa a variável x pelo tamanho do nome do registro mais 6
			x += 5 + registerStatus[i].registerName.length() + 1;
		}
		// Move o cursor para a posição (x, y+2) na tela
		gotoxy(x, y+2);

		// Imprime na tela uma barra vertical
		cout << "|";

		// Move o cursor para a posição (x, y+3) na tela
		gotoxy(x , y + 3);

		// Imprime na tela uma barra vertical
		cout << "|";

		// Imprime duas linhas em branco
		cout << endl << endl;

		// Imprime a string "O que aconteceu : "
		cout << "O que aconteceu : " << endl;

		// Imprime a razão do que aconteceu
		cout << reason;

	}
	Tomasulo()
	{

	}
};

int main(int argc, char* argv[])
{

	// Cria uma estrutura CONSOLE_FONT_INFOEX e define suas propriedades
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0; // Largura de cada caractere da fonte
	cfi.dwFontSize.Y = 18; // Altura
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"Consolas"); // Escolha sua fonte
	// Define a fonte atual do console para a fonte especificada na estrutura CONSOLE_FONT_INFOEX
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	// Define o modo de exibição do console para tela cheia
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
	COORD newSize = { 1000, 1000 };
	// Define o tamanho do buffer da tela do console

	// Obtém um identificador para o dispositivo de saída padrão (o console)
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	// Define os atributos de cor do texto do console para azul, verde, vermelho e intensidade
	SetConsoleTextAttribute(hStdOut, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);

	// Limpa a tela do console
	system("cls");

	// Cria uma instância da classe Tomasulo
	Tomasulo algorithm;

	// Carrega os dados do arquivo "source.txt" para a instância do algoritmo Tomasulo
	algorithm.loadDataFromFile("source.txt");

	// Chama a função Simulate na instância do algoritmo Tomasulo
	algorithm.Simulate();

	// Pausa a execução do programa e espera o usuário pressionar uma tecla
	system("pause");
}


void gotoxy(short x, short y)
{
	// Cria uma estrutura COORD e define suas propriedades
	COORD c = { x, y };
	// Define a posição do cursor no console
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
