import socket
import threading
import tkinter as tk
from tkinter import messagebox
import sys


class CheckersClient:
    def __init__(self, player_name):
        self.root = tk.Tk()
        self.root.title(f"Warcaby - {player_name}")
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.board = []
        self.selected = None
        self.player_name = player_name
        self.is_my_turn = False
        self.player_color = None  # Kolor pionków gracza (white/black)
        self.my_pieces = None  # Znak własnych pionków
        self.opponent_pieces = None  # Znak pionków przeciwnika

        # Połączenie z serwerem
        try:
            self.socket.connect(('127.0.0.1', 12345))
            connect_msg = f"CONNECT {self.player_name}"
            self.socket.send(connect_msg.encode())
        except Exception as e:
            messagebox.showerror("Błąd", f"Nie można połączyć z serwerem: {e}")
            sys.exit(1)

        # Tworzenie interfejsu użytkownika
        self.create_board()
        self.start_listening()

    def convert_coordinates(self, x, y):
        """Konwertuje współrzędne między perspektywami graczy."""
        if self.player_color == "black":
            # Dla czarnych graczy odwracamy współrzędne
            return 7 - x, 7 - y
        return x, y

    def create_board(self):
        """Tworzy planszę gry w Tkinter."""
        for i in range(8):
            row = []
            for j in range(8):
                bg_color = "#666666" if (i + j) % 2 == 1 else "#CCCCCC"
                button = tk.Button(
                    self.root,
                    width=4,
                    height=2,
                    bg=bg_color,
                    command=lambda x=i, y=j: self.button_click(x, y),
                )
                button.grid(row=i, column=j)
                row.append(button)
            self.board.append(row)

    def setup_initial_pieces(self):
        """Ustawia początkowe pionki na planszy."""
        for i in range(8):
            for j in range(8):
                self.board[i][j].configure(text="")  # Wyczyść planszę

        for i in range(8):
            for j in range(8):
                if (i + j) % 2 == 1:
                    if i < 3:
                        self.board[i][j].configure(text=self.opponent_pieces)
                    elif i > 4:
                        self.board[i][j].configure(text=self.my_pieces)

    def button_click(self, x, y):
        """Obsługuje kliknięcia na planszy."""
        if not self.is_my_turn:
            messagebox.showinfo("Czekaj", "Teraz nie twoja kolej!")
            return

        if self.selected is None:
            if self.board[x][y].cget("text") == self.my_pieces:
                self.selected = (x, y)
                self.board[x][y].configure(bg="yellow")
            return

        if self.selected:
            # Konwertujemy współrzędne do "normalnej" orientacji przed wysłaniem
            fromX, fromY = self.convert_coordinates(self.selected[0], self.selected[1])
            toX, toY = self.convert_coordinates(x, y)
            
            print(f"🎯 Klient wysyła ruch: ({fromX}, {fromY}) -> ({toX}, {toY})")
            move = f"MOVE {fromX} {fromY} {toX} {toY}"
            self.socket.send(move.encode())

            orig_color = "#666666" if (self.selected[0] + self.selected[1]) % 2 == 1 else "#CCCCCC"
            self.board[self.selected[0]][self.selected[1]].configure(bg=orig_color)
            self.selected = None

    def update_board(self, fromX, fromY, toX, toY):
        """Aktualizuje planszę po ruchu."""
        # Konwertujemy współrzędne do lokalnej perspektywy
        local_fromX, local_fromY = self.convert_coordinates(fromX, fromY)
        local_toX, local_toY = self.convert_coordinates(toX, toY)
        
        piece = self.board[local_fromX][local_fromY].cget("text")
        self.board[local_fromX][local_fromY].configure(text="")
        self.board[local_toX][local_toY].configure(text=piece)

    def start_listening(self):
        """Rozpoczyna wątek nasłuchiwania wiadomości z serwera."""
        def receive():
            while True:
                try:
                    data = self.socket.recv(1024).decode()
                    if not data:
                        break
                    
                    print(f"Otrzymano surowe dane: '{data}'")
                    messages = data.strip().split('\n')
                    print(f"Podzielono na wiadomości: {messages}")
                    
                    for message in messages:
                        if message:  # pomijamy puste linie
                            print(f"Przetwarzanie wiadomości: '{message}'")
                            self.process_message(message)

                except Exception as e:
                    print(f"Błąd połączenia: {e}")
                    break

        thread = threading.Thread(target=receive, daemon=True)
        thread.start()

    def process_message(self, data):
        """Przetwarza wiadomości otrzymane z serwera i aktualizuje stan gry."""
        parts = data.split()

        if not parts:
            return

        command = parts[0]

        if command == "COLOR":
            self.player_color = parts[1]

            # Ustawienie znaków pionków
            if self.player_color == "white":
                self.my_pieces = "●"
                self.opponent_pieces = "○"
            else:
                self.my_pieces = "○"
                self.opponent_pieces = "●"

            print(f"🔹 Twój kolor: {self.player_color} (Twoje pionki: {self.my_pieces})")
            self.root.after(0, self.setup_initial_pieces)

        elif command == "GAME_START":
            self.is_my_turn = (self.player_color == "white")
            print("🎉 Gra rozpoczęta!")
            if self.is_my_turn:
                print("▶ Twoja kolej!")
                self.root.after(0, lambda: messagebox.showinfo("Gra", "Gra się rozpoczęła! Twoja kolej!"))

        elif command == "MOVE_UPDATE":
            if len(parts) < 5:
                print("⚠ Błąd: Nieprawidłowy format MOVE_UPDATE")
                return

            _, fromX, fromY, toX, toY = parts
            fromX, fromY, toX, toY = int(fromX), int(fromY), int(toX), int(toY)

            print(f"\n🔹 Ruch wykonany: ({fromX}, {fromY}) -> ({toX}, {toY})")

            self.root.after(0, lambda: self.update_board(fromX, fromY, toX, toY))

        elif command == "YOUR_TURN":
            self.is_my_turn = True
            print("▶ Teraz twoja kolej!")
            self.root.after(0, lambda: messagebox.showinfo("Gra", "Twoja kolej!"))

        elif command == "WAIT_TURN":
            self.is_my_turn = False
            print("⏳ Czekaj na ruch przeciwnika...")

        elif command == "INVALID_MOVE":
            print("⚠ Nieprawidłowy ruch!")
            self.root.after(0, lambda: messagebox.showwarning("Nieprawidłowy", "Nieprawidłowy ruch!"))

        elif command == "NOT_YOUR_TURN":
            print("⚠ To nie jest twoja kolej!")
            self.root.after(0, lambda: messagebox.showinfo("Gra", "Teraz nie twoja kolej!"))

    def run(self):
        """Uruchamia główną pętlę Tkintera."""
        self.root.mainloop()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Użycie: python3 client.py [Player1/Player2]")
        sys.exit(1)
    client = CheckersClient(sys.argv[1])
    client.run()
