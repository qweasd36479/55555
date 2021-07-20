uint64_t ReadAddressFromGame(uint64_t AddressToRead)
			{
				static uint64_t RandomPageLocation = 0;
 
				// Find codecave << shit version btw lol
				if(!RandomPageLocation)
					while (true)
					{
						MEMORY_BASIC_INFORMATION Info;
						VirtualQueryEx(External::HANDLE, (void*)RandomPageLocation, &Info, sizeof(Info));
					
						RandomPageLocation = (uint64_t)Info.BaseAddress + Info.RegionSize;
 
						if (Info.AllocationProtect == PAGE_EXECUTE_READWRITE && Info.Protect == PAGE_EXECUTE_READWRITE && Info.State == MEM_COMMIT)
						{
							RandomPageLocation = (uint64_t)Info.BaseAddress;
 
							struct chunk_t
							{
								unsigned char Buff[64];
							};
							chunk_t A;
							ReadProcessMemory(External::HANDLE, (void*)RandomPageLocation, &A, sizeof(A), 0);
 
							bool Good = true;
 
							for (int i = 0; i < sizeof(A); i++)
							{
								if (A.Buff[i] != 0)
								{
									Good = false;
									break;
								}
								else if (A.Buff[i] != 0xCC)
								{
									Good = false;
									break;
								}
							}
 
							if (Good)
								break;
						}
					}
 
				// Assembly vv
				// mov rax, [rcx]
				// ret
				// Find this stub in valorant so we can call it so it reads the address and sets rax value to the real address.
				static uint64_t MemeStub = 0;
				if(!MemeStub)
					MemeStub = FindPattern(0, "\x48\x8B\x01\xC3", 5, 0xCC);
				if (!MemeStub)
					return 0;
 
				// .data function pointer which gets called every frame, censored ofc
				uint64_t DataPtr = External::Base + 0xDEADBEEF;
				
				static uint64_t OrigAddr = 0;
				if(!OrigAddr)
					ReadProcessMemory(External::HANDLE, (void*)DataPtr, &OrigAddr, sizeof(OrigAddr), 0);
 
				// Assembly vv
				// push rcx
				// push rax
				// mov rcx, 0xDEADBEEFCAFEBABE
				// call 0xDEADBEEFCAFEBABE
				// mov [0xDEADBEEFCAFEBABE], rax
				// pop rax
				// pop rcx
				// jmp 0xDEADBEEFCAFEBABE
				unsigned char ReadStub[] = {
					0x51, 0x50, 0x48, 0xB9, 0xBE, 0xBA, 0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE,
					0xFF, 0x15, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x08, 0xBE, 0xBA, 0xFE, 0xCA,
					0xEF, 0xBE, 0xAD, 0xDE, 0x48, 0xA3, 0xBE, 0xBA, 0xFE, 0xCA, 0xEF, 0xBE,
					0xAD, 0xDE, 0x58, 0x59, 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0xBE, 0xBA,
					0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE
				};
 
				unsigned char NullBuff[] = {
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00
				};
 
 
				*(uint64_t*)(&ReadStub[0] + 4) = AddressToRead;								// Address to read
				*(uint64_t*)(&ReadStub[0] + 20) = MemeStub;									// Stub
				*(uint64_t*)(&ReadStub[0] + 30) = RandomPageLocation + sizeof(ReadStub);	// Address to write
				*(uint64_t*)(&ReadStub[0] + 46) = OrigAddr;									// Original Location
				
				WriteProcessMemory(External::HANDLE, (void*)RandomPageLocation, &ReadStub[0], sizeof(ReadStub), 0);
				WriteProcessMemory(External::HANDLE, (void*)DataPtr, &RandomPageLocation, sizeof(DataPtr), 0);
 
				uint64_t ReadBuff = 0;
 
				while (true)
				{
					ReadProcessMemory(External::HANDLE, (void*)(RandomPageLocation + sizeof(ReadStub)), &ReadBuff, sizeof(ReadBuff), 0);
					if (ReadBuff)
						break;
				}
 
				WriteProcessMemory(External::HANDLE, (void*)DataPtr, &OrigAddr, sizeof(OrigAddr), 0);
				WriteProcessMemory(External::HANDLE, (void*)RandomPageLocation, &NullBuff[0], sizeof(NullBuff), 0);
 
				return ReadBuff;
			}
