import logging
from pygerber.gerberx3.api.v2 import GerberFile, Project

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

class GerberWrapper:
    def __init__(self):
        self.project = None
        logging.info("GerberWrapper initialized.")

    def parse_gerber_file(self, file_path):
        """Parse a Gerber file and return its content"""
        try:
            # Load Gerber file content
            with open(file_path, 'r') as f:
                content = f.read()

            # Parse the Gerber file
            gerber_file = GerberFile.from_str(content)

            logging.info(f"Successfully parsed Gerber file: {file_path}")
            return gerber_file
        except Exception as e:
            logging.error(f"Failed to parse Gerber file: {file_path}. Exception: {e}")
            raise

    def load_gerber_files(self, gerber_file_paths):
        """Load multiple Gerber files into the project."""
        try:
            loaded_files = [self.parse_gerber_file(file_path) for file_path in gerber_file_paths]
            self.project = Project(loaded_files)
            logging.info("Successfully initialized Project with all Gerber files.")
        except Exception as e:
            logging.error(f"Failed to load Gerber files. Exception: {e}")
            raise

    def clear_gerber_files(self):
        """Clears all loaded Gerber files."""
        self.project = None
        logging.info("Cleared all loaded Gerber files.")

    def render_to_png(self, output_path, dpmm=40):
        """Render the project to a PNG image."""
        if not self.project:
            logging.warning("No project initialized. Please load Gerber files first.")
            return

        try:
            self.project.parse().render_raster(output_path, dpmm=dpmm)
            logging.info(f"Successfully rendered Project to PNG: {output_path}")
        except Exception as e:
            logging.error(f"Failed to render Project to PNG. Exception: {e}")
            raise
