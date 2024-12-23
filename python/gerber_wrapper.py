
import logging
from pygerber.gerber_layer import GerberLayer
from pygerber.drill_layer import DrillLayer
from pygerber.renderers.svg import SvgLayerRenderer

logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')

class GerberWrapper:
    def __init__(self):
        """
        Initializes the GerberWrapper instance by setting up lists to store
        Gerber and Drill layers.
        """
        self.gerber_layers = []
        self.drill_layers = []
        logging.info("GerberWrapper initialized.")

    def load_gerber_file(self, file_path):
        """
        Loads a Gerber or Drill file based on its extension.

        Args:
            file_path (str): The path to the Gerber or Drill file.

        Returns:
            bool: True if the file was loaded successfully, False otherwise.
        """
        logging.info(f"Attempting to load file from: {file_path}")

        if not isinstance(file_path, str):
            logging.error("File path must be a string.")
            return False

        if not file_path.lower().endswith(('.gbr', '.gerber', '.drl')):
            logging.error(f"Unsupported file extension for file: {file_path}")
            return False

        try:
            if file_path.lower().endswith(('.gbr', '.gerber')):
                gerber = GerberLayer()
                gerber.read(file_path)
                self.gerber_layers.append(gerber)
                logging.info(f"Successfully loaded Gerber file: {file_path}")
            elif file_path.lower().endswith('.drl'):
                drill = DrillLayer()
                drill.read(file_path)
                self.drill_layers.append(drill)
                logging.info(f"Successfully loaded Drill file: {file_path}")
            return True
        except Exception as e:
            logging.error(f"Failed to load file '{file_path}': {e}")
            return False

    def clear_gerber_files(self):
        """
        Clears all loaded Gerber and Drill files from the wrapper.
        """
        self.gerber_layers.clear()
        self.drill_layers.clear()
        logging.info("Cleared all loaded Gerber and Drill files.")

    def get_loaded_files(self):
        """
        Retrieves a list of all loaded Gerber and Drill file paths.

        Returns:
            dict: A dictionary containing lists of Gerber and Drill file paths.
        """
        gerber_paths = [layer.file_path for layer in self.gerber_layers if hasattr(layer, 'file_path')]
        drill_paths = [drill.file_path for drill in self.drill_layers if hasattr(drill, 'file_path')]
        return {
            "Gerber Files": gerber_paths,
            "Drill Files": drill_paths
        }
